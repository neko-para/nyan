#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int dup(int fd) {
    auto fileObjPtr = task::__scheduler->__current->__file.getFile(fd);
    if (!fileObjPtr) {
        return -SYS_EBADF;
    }

    int newFd;
    if (auto fileObjSlotPtr = task::__scheduler->__current->__file.getFileSlot(newFd)) {
        *fileObjSlotPtr = *fileObjPtr;
        return newFd;
    }

    return -SYS_EMFILE;
}

}  // namespace nyan::syscall
