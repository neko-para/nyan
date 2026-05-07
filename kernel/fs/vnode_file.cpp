#include "vnode_file.hpp"

namespace nyan::fs {

Result<ssize_t> VNodeFileObj::read(void* buf, size_t size) noexcept {
    auto ret = __try(__vnode->read(buf, size, __offset));
    __offset += ret;
    return ret;
}

Result<ssize_t> VNodeFileObj::write(const void* buf, size_t size) noexcept {
    // TODO: O_APPEND
    auto ret = __try(__vnode->write(buf, size, __offset));
    __offset += ret;
    return ret;
}

Result<int> VNodeFileObj::ioctl(uint32_t req, uint32_t param) noexcept {
    return __vnode->ioctl(req, param);
}

}  // namespace nyan::fs
