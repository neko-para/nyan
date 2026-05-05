#include "vnode_file.hpp"

namespace nyan::fs {

ssize_t VNodeFileObj::read(void* buf, size_t size) noexcept {
    auto ret = __vnode->read(buf, size, __offset);
    if (ret > 0) {
        __offset += ret;
    }
    return ret;
}

ssize_t VNodeFileObj::write(const void* buf, size_t size) noexcept {
    // TODO: O_APPEND
    auto ret = __vnode->write(buf, size, __offset);
    if (ret > 0) {
        __offset += ret;
    }
    return ret;
}

int VNodeFileObj::ioctl(uint32_t req, uint32_t param) noexcept {
    return __vnode->ioctl(req, param);
}

}  // namespace nyan::fs