#pragma once

#include <string>
#include <vector>

#include "../../lib/function.hpp"
#include "../device.hpp"
#include "../filesystem.hpp"
#include "../super_block.hpp"
#include "../vnode.hpp"

namespace nyan::fs {

struct RamFSSuperBlock : public SuperBlock {
    uint64_t __counter{};
};

struct RamFSVNode : public VNode {
    RamFSVNode(VNodeType type, lib::Ref<SuperBlock> sb, uint32_t mode);

    RamFSSuperBlock* super_block() const noexcept { return __super_block.as<RamFSSuperBlock>(); }
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

    RamFSDirectoryVNode(lib::Ref<SuperBlock> super_block, uint32_t mode)
        : RamFSVNode(VNT_Directory, super_block, mode) {}

    std::vector<Entry>::iterator __find(std::string_view name) noexcept;
    bool __exists(std::string_view name) noexcept;

    virtual Result<lib::Ref<VNode>> lookup(std::string_view name) noexcept override;
    virtual Result<int> readdir(dirent* buf, size_t size, off_t* offset) noexcept override;
    virtual Result<> mkdir(std::string_view name, uint32_t mode) noexcept override;
    virtual Result<> create(std::string_view name, uint32_t mode) noexcept override;
    virtual Result<> link(std::string_view name, lib::Ref<VNode> target) noexcept override;
    virtual Result<> unlink(std::string_view name) noexcept override;
    virtual Result<> symlink(std::string_view name, std::string_view target) noexcept override;

    virtual Result<> stat(struct stat* buf) noexcept override;
};

struct RamFSFileVNode : public RamFSVNode {
    std::vector<uint8_t> __data;

    RamFSFileVNode(lib::Ref<SuperBlock> super_block, uint32_t mode) : RamFSVNode(VNT_Regular, super_block, mode) {}

    virtual Result<ssize_t> read(void* buf, size_t size, off_t offset) noexcept override;
    virtual Result<ssize_t> write(const void* buf, size_t size, off_t offset) noexcept override;
    virtual Result<> truncate(off_t length) noexcept override;

    virtual Result<> stat(struct stat* buf) noexcept override;
};

struct RamFSSymlinkVNode : public RamFSVNode {
    std::string __target;
    lib::function<Result<std::string>()> __dynamic_target;

    RamFSSymlinkVNode(std::string target, lib::Ref<SuperBlock> super_block, uint32_t mode)
        : RamFSVNode(VNT_Symlink, super_block, mode), __target(std::move(target)) {}
    RamFSSymlinkVNode(lib::function<Result<std::string>()> dynamic_target,
                      lib::Ref<SuperBlock> super_block,
                      uint32_t mode)
        : RamFSVNode(VNT_Symlink, super_block, mode), __dynamic_target(std::move(dynamic_target)) {}

    virtual Result<std::string> readlink() noexcept override;

    virtual Result<> stat(struct stat* buf) noexcept override;
};

struct RamFSCharDevVNode : public RamFSVNode {
    CharDevice* __device;

    RamFSCharDevVNode(CharDevice* device, lib::Ref<SuperBlock> super_block, uint32_t mode)
        : RamFSVNode(VNT_CharDevice, super_block, mode), __device(device) {}

    virtual Result<ssize_t> read(void* buf, size_t size, off_t offset) noexcept override;
    virtual Result<ssize_t> write(const void* buf, size_t size, off_t offset) noexcept override;

    virtual Result<> stat(struct stat* buf) noexcept override;
    virtual Result<> ioctl(unsigned cmd, uint32_t arg) noexcept override;
};

struct RamFS : public FileSystem {
    virtual lib::Ref<MountEntry> mount(Device* device, const char* options) noexcept override;
};

}  // namespace nyan::fs
