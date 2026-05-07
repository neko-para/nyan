#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int dup(int fd) {
    auto fileObj = __try(task::__scheduler->__current->__file.getFile(fd));

    int newFd;
    auto fileObjSlotPtr = __try(task::__scheduler->__current->__file.findFileSlot(newFd));

    *fileObjSlotPtr = fileObj;
    return newFd;
}

}  // namespace nyan::syscall
