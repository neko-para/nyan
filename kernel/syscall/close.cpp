#include <nyan/syscall.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

int close(int fd) {
    auto fileObjPtr = task::currentTask->__file.getFile(fd);
    if (!fileObjPtr) {
        return -SYS_EBADF;
    }
    (*fileObjPtr) = {};
    return 0;
}

}  // namespace nyan::syscall
