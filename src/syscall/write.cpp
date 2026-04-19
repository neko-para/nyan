#include <nyan/syscall.h>

#include "../arch/utils.hpp"
#include "../console/entry.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

ssize_t write(int fd, const void* buf, size_t size) {
    if (!buf) {
        return -SYS_EFAULT;
    }
    if (size > INT_MAX) {
        return -SYS_EINVAL;
    }
    if (fd == 1) {
        if (auto tty = task::currentTask->tty) {
            tty->puts(static_cast<const char*>(buf), size);
            return size;
        } else {
            return -SYS_EBADF;
        }
    } else if (fd == 2) {
        arch::kputs(static_cast<const char*>(buf), size);
        return size;
    } else {
        return -SYS_EBADF;
    }
}

}  // namespace nyan::syscall
