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
    };

    std::vector<Entry> __entries;

    RamFSDirectoryVNode(SuperBlock* super_block, uint32_t mode) : RamFSVNode(VNT_Directory, super_block, mode) {}

    std::vector<Entry>::iterator __find(const char* name) noexcept;

    virtual lib::Ref<VNode> lookup(const char* name) noexcept override;
    virtual int readdir(dirent* buf, size_t size, off_t* offset) noexcept override;
    virtual int mkdir(const char* name, uint32_t mode) noexcept override;
    virtual int touch(const char* name, uint32_t mode) noexcept override;
    virtual int link(const char* name, lib::Ref<VNode> target) noexcept override;
    virtual int unlink(const char* name) noexcept override;

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
