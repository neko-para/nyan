#include "device.hpp"

#include <nyan/errno.h>
#include <sys/ioctl.h>

#include "../task/pid.hpp"
#include "buffer.hpp"
#include "tty.hpp"

namespace nyan::console {

Result<ssize_t> TtyDevice::read(void* buf, size_t size) noexcept {
    auto guard = __try(__tty->syncWaitInput());

    auto result = std::min(__tty->__input_buffer.size(), size);
    std::copy_n(__tty->__input_buffer.data(), result, static_cast<uint8_t*>(buf));
    __tty->__input_buffer.erase(0, result);
    return result;
}

Result<ssize_t> TtyDevice::write(const void* buf, size_t size) noexcept {
    __tty->puts(static_cast<const char*>(buf), size);
    return size;
}

Result<> TtyDevice::ioctl(uint32_t req, uint32_t param) noexcept {
    switch (req) {
        case TIOCSPGRP: {
            if (!param) {
                return SYS_EFAULT;
            }
            auto pid = *reinterpret_cast<pid_t*>(param);
            if (pid && !task::findTask(pid)) {
                return SYS_EINVAL;
            }
            // TODO: check same session
            __tty->__foreground_pid = pid;
            return {};
        }
        case TIOCGWINSZ:
            winsize* ptr = reinterpret_cast<winsize*>(param);
            if (!ptr) {
                return SYS_EFAULT;
            }
            ptr->ws_row = __height;
            ptr->ws_col = __width;
            ptr->ws_xpixel = 0;
            ptr->ws_ypixel = 0;
            return {};
    }
    return SYS_ENOTTY;
}

}  // namespace nyan::console
