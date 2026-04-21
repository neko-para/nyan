#include <nyan/syscall.h>

#include "../arch/guard.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int dup2(int fd, int newFd) {
    if (fd < 0 || static_cast<size_t>(fd) >= task::MAXFD) {
        return -SYS_EBADF;
    }
    if (fd == newFd) {
        return fd;
    }
    if (newFd < 0 || static_cast<size_t>(newFd) >= task::MAXFD) {
        return -SYS_EBADF;
    }
    arch::InterruptGuard guard;
    auto& fileObj = task::currentTask->fdTable[fd];
    if (!fileObj) {
        return -SYS_EBADF;
    }
    auto& fileObjSlot = task::currentTask->fdTable[newFd];
    fileObjSlot = fileObj;
    return newFd;
}

}  // namespace nyan::syscall
