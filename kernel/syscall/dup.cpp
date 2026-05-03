#include <nyan/syscall.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

int dup(int fd) {
    auto fileObjPtr = task::currentTask->__file.getFile(fd);
    if (!fileObjPtr) {
        return -SYS_EBADF;
    }

    int newFd;
    if (auto fileObjSlotPtr = task::currentTask->__file.getFileSlot(newFd)) {
        *fileObjSlotPtr = *fileObjPtr;
        return newFd;
    }

    return -SYS_EMFILE;
}

}  // namespace nyan::syscall
