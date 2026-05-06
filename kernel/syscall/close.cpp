#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int close(int fd) {
    auto fileObjPtr = task::__scheduler->__current->__file.getFileSlot(fd);
    if (!fileObjPtr) {
        return -SYS_EBADF;
    }
    (*fileObjPtr) = {};
    return 0;
}

}  // namespace nyan::syscall
