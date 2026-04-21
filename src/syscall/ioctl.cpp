#include <nyan/syscall.h>
#include <sys/ioctl.h>

#include "../arch/guard.hpp"
#include "../console/entry.hpp"
#include "../task/pid.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int ioctl(int fd, uint32_t request, uint32_t param) {
    arch::InterruptGuard guard;
    switch (request) {
        case TIOCSPGRP:
            // TODO: check fd, needs to be controlling terminal
            if (fd == 0) {
                if (!task::currentTask->tty) {
                    return -SYS_ENOTTY;
                }
                if (param && !task::findTask(param)) {
                    return -SYS_EINVAL;
                }
                // TODO: check same session
                task::currentTask->tty->foregroundPid = param;
                return 0;
            } else {
                return -SYS_EBADF;
            }
    }
    return -SYS_ENOSYS;
}

}  // namespace nyan::syscall
