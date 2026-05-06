#include <nyan/syscall.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"
#include "utils.hpp"

namespace nyan::syscall {

ssize_t read(int fd, void* buf, size_t size) {
    if (size > INT_MAX) {
        return -SYS_EINVAL;
    }
    if (!utils::validateWrite(buf, size)) {
        return -SYS_EFAULT;
    }
    auto fileObj = task::__scheduler->__current->__file.getFile(fd);
    if (!fileObj) {
        return -SYS_EBADF;
    }
    return fileObj->read(buf, size);
}

}  // namespace nyan::syscall
