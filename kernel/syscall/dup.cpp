#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int dup(int fd) {
    auto fileObj = task::__scheduler->__current->__file.getFile(fd);
    if (!fileObj) {
        return -SYS_EBADF;
    }

    int newFd;
    if (auto fileObjSlotPtr = task::__scheduler->__current->__file.findFileSlot(newFd)) {
        *fileObjSlotPtr = fileObj;
        return newFd;
    }

    return -SYS_EMFILE;
}

}  // namespace nyan::syscall
