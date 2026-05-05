#include "pipe.hpp"

#include <fcntl.h>
#include <nyan/errno.h>
#include <signal.h>
#include <string.h>

#include "../arch/print.hpp"
#include "../task/scheduler.hpp"

namespace nyan::fs {

PipeState::PipeState() {
    __buffer = new char[__pipe_buffer_size];
}

PipeState::~PipeState() {
    delete[] __buffer;
}

std::optional<arch::InterruptGuard> PipeState::syncWaitForRead() noexcept {
    while (true) {
        arch::InterruptGuard guard;
        if (!empty() || !__write_alive) {
            return guard;
        }
        if (__read_wait.wait(task::BlockReason::BR_WaitInput) == task::WakeReason::WR_Signal) {
            return std::nullopt;
        }
    }
}

std::optional<arch::InterruptGuard> PipeState::syncWaitForWrite() noexcept {
    while (true) {
        arch::InterruptGuard guard;
        if (!full() || !__read_alive) {
            return guard;
        }
        if (__write_wait.wait(task::BlockReason::BR_WaitInput) == task::WakeReason::WR_Signal) {
            return std::nullopt;
        }
    }
}

void PipeState::onReadClosed() noexcept {
    __read_alive = false;
    __write_wait.wakeAll(task::WakeReason::WR_Normal);
}

void PipeState::onWriteClosed() noexcept {
    __write_alive = false;
    __read_wait.wakeAll(task::WakeReason::WR_Normal);
}

ssize_t PipeState::read(void* buf, size_t sz) noexcept {
    auto ptr = static_cast<uint8_t*>(buf);
    auto cur = ptr;

    auto guard = syncWaitForRead();
    if (!guard) {
        return -SYS_EINTR;
    }

    if (empty() && !__write_alive) {
        return 0;
    }

    size_t len = std::min(size(), sz);
    auto left = __tail & __pipe_buffer_mask;
    auto right = left + len;
    if (right > __pipe_buffer_size) {
        auto firstLen = __pipe_buffer_size - left;
        auto restLen = len - firstLen;
        memcpy(cur, &__buffer[left], firstLen);
        memcpy(cur + firstLen, &__buffer[0], restLen);
    } else {
        memcpy(cur, &__buffer[left], len);
    }
    __tail += len;
    __write_wait.wakeAll(task::WakeReason::WR_Normal);
    return len;
}

ssize_t PipeState::write(const void* buf, size_t sz) noexcept {
    auto ptr = static_cast<const uint8_t*>(buf);
    auto cur = ptr;

    auto guard = syncWaitForWrite();
    if (!guard) {
        return -SYS_EINTR;
    }

    if (!__read_alive) {
        task::__scheduler->raise(SIGPIPE);
        return -SYS_EPIPE;
    }

    size_t len = std::min(__pipe_buffer_size - size(), sz);
    auto left = __head & __pipe_buffer_mask;
    auto right = left + len;
    if (right > __pipe_buffer_size) {
        auto firstLen = __pipe_buffer_size - left;
        auto restLen = len - firstLen;
        memcpy(&__buffer[left], cur, firstLen);
        memcpy(&__buffer[0], cur + firstLen, restLen);
    } else {
        memcpy(&__buffer[left], cur, len);
    }
    __head += len;
    __read_wait.wakeAll(task::WakeReason::WR_Normal);
    return len;
}

ssize_t PipeObj::read(void* buf, size_t size) noexcept {
    return __state->read(buf, size);
}

ssize_t PipeObj::write(const void* buf, size_t size) noexcept {
    return __state->write(buf, size);
}

int PipeObj::ioctl(uint32_t, uint32_t) noexcept {
    return -SYS_ENOTTY;
}

void PipeObj::onFdClose() noexcept {
    switch (__mode & O_ACCMODE) {
        case O_RDONLY:
            __state->onReadClosed();
            break;
        case O_WRONLY:
            __state->onWriteClosed();
            break;
        default:
            arch::kfatal("pipe shouldn't use O_RDWR");
    }
}

}  // namespace nyan::fs
