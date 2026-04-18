#include <nyan/syscall.h>

#include "../task/guard.hpp"
#include "../tty/entry.hpp"

namespace nyan::syscall {

ssize_t read(int fd, void* buf, size_t size) {
    if (fd != 0) {
        return -SYS_EBADF;
    }
    if (!buf) {
        return -SYS_EFAULT;
    }
    if (size > INT_MAX) {
        return -SYS_EINVAL;
    }
    if (fd == 0) {
        tty::activeTty->syncWaitInput();
        task::InterruptGuard guard;
        return tty::activeTty->inputBuffer.popSome(static_cast<uint8_t*>(buf), size);
    } else {
        return -SYS_EBADF;
    }
}

}  // namespace nyan::syscall
