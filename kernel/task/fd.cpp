#include "fd.hpp"

#include <nyan/errno.h>
#include <nyan/ioctls.h>

#include "../arch/debug.hpp"
#include "../console/entry.hpp"
#include "pid.hpp"

namespace nyan::task {

ssize_t DebugConObj::read(void* buf, size_t size) const noexcept {
    std::ignore = buf;
    std::ignore = size;
    return -SYS_EBADF;
}

ssize_t DebugConObj::write(const void* buf, size_t size) const noexcept {
    arch::kputs(static_cast<const char*>(buf), size);
    return size;
}

int DebugConObj::ioctl(uint32_t req, uint32_t param) const noexcept {
    std::ignore = req;
    std::ignore = param;
    return -SYS_ENOTTY;
}

ssize_t TtyObj::read(void* buf, size_t size) const noexcept {
    if (auto guard = tty->syncWaitInput()) {
        auto result = std::min(tty->inputBuffer.size(), size);
        std::copy_n(tty->inputBuffer.data(), result, static_cast<uint8_t*>(buf));
        tty->inputBuffer.erase(0, result);
        return result;
    } else {
        return -SYS_EINTR;
    }
}

ssize_t TtyObj::write(const void* buf, size_t size) const noexcept {
    tty->puts(static_cast<const char*>(buf), size);
    return size;
}

int TtyObj::ioctl(uint32_t req, uint32_t param) const noexcept {
    switch (req) {
        case TIOCSPGRP:
            if (param && !findTask(param)) {
                return -SYS_EINVAL;
            }
            // TODO: check same session
            tty->foregroundPid = param;
            return 0;
    }
    return -SYS_ENOTTY;
}

}  // namespace nyan::task
