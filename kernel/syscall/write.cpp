#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

ssize_t write(int fd, const void* buf, size_t size) {
    if (!buf) {
        return -SYS_EFAULT;
    }
    if (size > INT_MAX) {
        return -SYS_EINVAL;
    }
    auto fileObjPtr = task::__scheduler->__current->__file.getFile(fd);
    if (!fileObjPtr) {
        return -SYS_EBADF;
    }
    return (*fileObjPtr)->write(buf, size);
}

}  // namespace nyan::syscall
