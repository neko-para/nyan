#include <nyan/syscall.h>
#include <sys/ioctl.h>

#include "../arch/guard.hpp"
#include "../console/entry.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int ioctl(int fd, uint32_t request, uint32_t param) {
    arch::InterruptGuard guard;
    switch (request) {
        case TIOCSPGRP:
            if (fd == 0) {
                if (!task::currentTask->tty) {
                    return -SYS_ENOTTY;
                }
                task::currentTask->tty->foregroundPid = param;
                return 0;
            } else {
                return -SYS_EBADF;
            }
    }
    return -SYS_ENOSYS;
}

}  // namespace nyan::syscall
