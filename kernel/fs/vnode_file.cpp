#include "vnode_file.hpp"

#include <fcntl.h>

namespace nyan::fs {

Result<ssize_t> VNodeFileObj::read(void* buf, size_t size) noexcept {
    auto ret = __try(__vnode->read(buf, size, __offset));
    __offset += ret;
    return ret;
}

Result<ssize_t> VNodeFileObj::write(const void* buf, size_t size) noexcept {
    if (__mode & O_APPEND) {
        __try
            (seek(0, SEEK_END));
    }
    auto ret = __try(__vnode->write(buf, size, __offset));
    __offset += ret;
    return ret;
}

Result<> VNodeFileObj::ioctl(unsigned cmd, uint32_t arg) noexcept {
    return __vnode->ioctl(cmd, arg);
}

Result<off_t> VNodeFileObj::seek(off_t offset, int whence) noexcept {
    if (__vnode->isPipe()) {
        return SYS_ESPIPE;
    }

    off_t newOff;
    switch (whence) {
        case SEEK_SET:
            newOff = 0;
            break;
        case SEEK_CUR:
            if (__builtin_add_overflow(__offset, offset, &newOff)) {
                return SYS_EOVERFLOW;
            }
            break;
        case SEEK_END:
            if (__vnode->isDirectory()) {
                return SYS_EINVAL;
            }
            struct stat info;
            __try
                (__vnode->stat(&info));
            if (__builtin_add_overflow(info.st_size, offset, &newOff)) {
                return SYS_EOVERFLOW;
            }
            break;
        default:
            return SYS_EINVAL;
    }

    if (newOff < 0) {
        return SYS_EINVAL;
    }

    __offset = newOff;
    return 0;
}

}  // namespace nyan::fs
