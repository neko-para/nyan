#include <nyan/syscall.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

ssize_t read(int fd, void* buf, size_t size) {
    if (!buf) {
        return -SYS_EFAULT;
    }
    if (size > INT_MAX) {
        return -SYS_EINVAL;
    }
    auto fileObjPtr = task::currentTask->__file.getFile(fd);
    if (!fileObjPtr) {
        return -SYS_EBADF;
    }
    return (*fileObjPtr)->read(buf, size);
}

}  // namespace nyan::syscall
