#pragma once

#include <string>
#include <vector>

#include "../filesystem.hpp"
#include "../super_block.hpp"
#include "../vnode.hpp"

namespace nyan::fs {

struct RamFSSuperBlock : public SuperBlock {
    uint64_t __counter{};
};

struct RamFSVNode : public VNode {
    RamFSVNode(VNodeType type, SuperBlock* sb, uint32_t mode);

    RamFSSuperBlock* super_block() const noexcept { return static_cast<RamFSSuperBlock*>(__super_block); }
};

struct RamFSDirectoryVNode : public RamFSVNode {
    struct Entry {
        std::string __name;
        lib::Ref<VNode> __vnode;

        Entry() = default;
        template <typename T, typename U>
        Entry(T&& name, U&& vnode) : __name(std::forward<T>(name)), __vnode(std::forward<U>(vnode)) {}
    };

    std::vector<Entry> __entries;

    RamFSDirectoryVNode(SuperBlock* super_block, uint32_t mode) : RamFSVNode(VNT_Directory, super_block, mode) {}

    std::vector<Entry>::iterator __find(std::string_view name) noexcept;

    virtual lib::Ref<VNode> lookup(std::string_view name) noexcept override;
    virtual int readdir(dirent* buf, size_t size, off_t* offset) noexcept override;
    virtual int mkdir(std::string_view name, uint32_t mode) noexcept override;
    virtual int create(std::string_view name, uint32_t mode) noexcept override;
    virtual int link(std::string_view name, lib::Ref<VNode> target) noexcept override;
    virtual int unlink(std::string_view name) noexcept override;

    virtual int stat(struct stat* buf) noexcept override;
};

struct RamFSFileVNode : public RamFSVNode {
    std::vector<uint8_t> __data;

    RamFSFileVNode(SuperBlock* super_block, uint32_t mode) : RamFSVNode(VNT_Regular, super_block, mode) {}

    virtual ssize_t read(void* buf, size_t size, off_t offset) noexcept override;
    virtual ssize_t write(const void* buf, size_t size, off_t offset) noexcept override;
    virtual int truncate(off_t length) noexcept override;

    virtual int stat(struct stat* buf) noexcept override;
};

struct RamFS : public FileSystem {
    virtual SuperBlock* mount(Device* device, const char* options) noexcept override;
};

}  // namespace nyan::fs
