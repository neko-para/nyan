#include <nyan/syscall.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

int dup2(int fd, int newFd) {
    auto fileObjPtr = task::currentTask->__file.getFile(fd);
    if (!fileObjPtr) {
        return -SYS_EBADF;
    }
    if (fd == newFd) {
        return fd;
    }

    auto fileObjSlotPtr = task::currentTask->__file.getFile(newFd);
    if (!fileObjPtr) {
        return -SYS_EBADF;
    }
    *fileObjSlotPtr = *fileObjPtr;
    return newFd;
}

}  // namespace nyan::syscall
