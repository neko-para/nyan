#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int dup2(int fd, int newFd) {
    auto fileObj = task::__scheduler->__current->__file.getFile(fd);
    if (!fileObj) {
        return -SYS_EBADF;
    }
    if (fd == newFd) {
        return fd;
    }

    auto fileObjSlotPtr = task::__scheduler->__current->__file.findFileSlot(newFd);
    if (!fileObjSlotPtr) {
        return -SYS_EBADF;
    }
    *fileObjSlotPtr = fileObj;
    return newFd;
}

}  // namespace nyan::syscall
