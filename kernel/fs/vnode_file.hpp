#pragma once

#include "file.hpp"
#include "vnode.hpp"

namespace nyan::fs {

struct VNodeFileObj : public FileObj {
    lib::Ref<VNode> __vnode;

    VNodeFileObj(lib::Ref<VNode> vnode, uint32_t mode) noexcept : FileObj(mode), __vnode(vnode) {}

    virtual ssize_t read(void* buf, size_t size) noexcept override;
    virtual ssize_t write(const void* buf, size_t size) noexcept override;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept override;
};

}  // namespace nyan::fs