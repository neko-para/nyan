#include "file.hpp"

#include <nyan/errno.h>
#include <sys/ioctl.h>

#include "../task/pid.hpp"
#include "tty.hpp"

namespace nyan::console {

ssize_t TtyObj::read(void* buf, size_t size) noexcept {
    if (auto guard = tty->syncWaitInput()) {
        auto result = std::min(tty->inputBuffer.size(), size);
        std::copy_n(tty->inputBuffer.data(), result, static_cast<uint8_t*>(buf));
        tty->inputBuffer.erase(0, result);
        return result;
    } else {
        return -SYS_EINTR;
    }
}

ssize_t TtyObj::write(const void* buf, size_t size) noexcept {
    tty->puts(static_cast<const char*>(buf), size);
    return size;
}

int TtyObj::ioctl(uint32_t req, uint32_t param) noexcept {
    switch (req) {
        case TIOCSPGRP: {
            if (!param) {
                return -SYS_EFAULT;
            }
            auto pid = *reinterpret_cast<pid_t*>(param);
            if (pid && !task::findTask(pid)) {
                return -SYS_EINVAL;
            }
            // TODO: check same session
            tty->foregroundPid = param;
            return 0;
        }
        case TIOCGWINSZ:
            winsize* ptr = reinterpret_cast<winsize*>(param);
            if (!ptr) {
                return -SYS_EFAULT;
            }
            ptr->ws_row = 25;
            ptr->ws_col = 80;
            ptr->ws_xpixel = 0;
            ptr->ws_ypixel = 0;
            return 0;
    }
    return -SYS_ENOTTY;
}

}  // namespace nyan::console
