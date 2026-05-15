#pragma once

#include "file.hpp"
#include "vnode.hpp"

namespace nyan::fs {

struct VNodeFileObj : public FileObj {
    lib::Ref<VNode> __vnode;

    VNodeFileObj(lib::Ref<VNode> vnode, uint32_t mode) noexcept : FileObj(mode), __vnode(vnode) {}

    virtual lib::Ref<VNode> getVNode() noexcept override { return __vnode; }

    virtual Result<ssize_t> read(void* buf, size_t size) noexcept override;
    virtual Result<ssize_t> write(const void* buf, size_t size) noexcept override;
    virtual Result<> stat(struct stat* buf) noexcept override;
    virtual Result<> ioctl(uint32_t req, uint32_t param) noexcept override;
    virtual Result<off_t> seek(off_t offset, int whence) noexcept override;
};

}  // namespace nyan::fs
