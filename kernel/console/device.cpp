#include "device.hpp"

#include <nyan/errno.h>
#include <sys/ioctl.h>

#include "../arch/print.hpp"
#include "../task/mod.hpp"
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

Result<> TtyDevice::ioctl(unsigned cmd, uint32_t arg) noexcept {
    switch (cmd) {
        case TCGETS:
            *__try(task::checkW<termios>(arg)) = __tty->__config;
            return {};
        case TCSETS:
            __tty->__config = *__try(task::checkR<termios>(arg));
            return {};
        case TIOCGPGRP:
            *__try(task::checkW<pid_t>(arg)) = __tty->__foreground_pgid;
            return {};
        case TIOCSPGRP: {
            auto pgid = *__try(task::checkR<pid_t>(arg));
            __try
                (task::findTaskGroup(pgid));
            __tty->__foreground_pgid = pgid;
            arch::kprint("tty switch foregroup to {}", pgid);
            return {};
        }
        case TIOCGWINSZ: {
            winsize* ptr = __try(task::checkW<winsize>(arg));
            ptr->ws_row = __height;
            ptr->ws_col = __width;
            ptr->ws_xpixel = 0;
            ptr->ws_ypixel = 0;
            return {};
        }
    }
    return SYS_ENOTTY;
}

}  // namespace nyan::console
