#include "fd.hpp"

#include <fcntl.h>
#include <nyan/errno.h>

namespace nyan::fs {

FdObj::~FdObj() {
    __file->onFdClose();
}

ssize_t FdObj::read(void* buf, size_t size) const noexcept {
    auto mode = __file->__mode & O_ACCMODE;
    if (mode != O_RDONLY && mode != O_RDWR) {
        return -SYS_EBADF;
    }
    return __file->read(buf, size);
}

ssize_t FdObj::write(const void* buf, size_t size) const noexcept {
    auto mode = __file->__mode & O_ACCMODE;
    if (mode != O_WRONLY && mode != O_RDWR) {
        return -SYS_EBADF;
    }
    return __file->write(buf, size);
}

int FdObj::ioctl(uint32_t req, uint32_t param) const noexcept {
    return __file->ioctl(req, param);
}

}  // namespace nyan::fs
