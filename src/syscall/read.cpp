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
    if (fd < 0 || static_cast<size_t>(fd) >= task::MAXFD) {
        return -SYS_EBADF;
    }
    const auto& fileObj = task::currentTask->fdTable[fd];
    if (!fileObj) {
        return -SYS_EBADF;
    }
    auto mode = fileObj->mode & O_ACCMODE;
    if (mode != O_RDONLY && mode != O_RDWR) {
        return -SYS_EBADF;
    }
    return fileObj->read(buf, size);
}

}  // namespace nyan::syscall
