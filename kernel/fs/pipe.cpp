#include "pipe.hpp"

#include <fcntl.h>
#include <nyan/errno.h>
#include <signal.h>
#include <string.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::fs {

PipeObj::PipeObj() {
    __buffer = new char[4096];
}

PipeObj::~PipeObj() {
    delete[] __buffer;
}

std::optional<arch::InterruptGuard> PipeObj::syncWaitForRead() noexcept {
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

std::optional<arch::InterruptGuard> PipeObj::syncWaitForWrite() noexcept {
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

ssize_t PipeObj::read(void* buf, size_t sz) noexcept {
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
    auto left = __tail & 0xFFF;
    auto right = left + len;
    if (right > 0x1000) {
        auto firstLen = 0x1000 - left;
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

ssize_t PipeObj::write(const void* buf, size_t sz) noexcept {
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

    size_t len = std::min(PipeBufferSize - size(), sz);
    auto left = __head & 0xFFF;
    auto right = left + len;
    if (right > 0x1000) {
        auto firstLen = 0x1000 - left;
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

int PipeObj::ioctl(uint32_t req, uint32_t param) noexcept {
    std::ignore = req;
    std::ignore = param;
    return -SYS_ENOTTY;
}

void PipeObj::onFdClose(uint32_t mode) noexcept {
    switch (mode & O_ACCMODE) {
        case O_RDONLY:
            __read_alive = false;
            __write_wait.wakeAll(task::WakeReason::WR_Normal);
            break;
        case O_WRONLY:
            __write_alive = false;
            __read_wait.wakeAll(task::WakeReason::WR_Normal);
            break;
        default:
            arch::kfatal("pipe shouldn't use O_RDWR");
    }
}

}  // namespace nyan::fs
