#pragma once

#include <dirent.h>
#include <nyan/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string_view>

#include "../lib/shared.hpp"
#include "forward.hpp"

namespace nyan::fs {

enum VNodeType {
    VNT_Regular = DT_REG,
    VNT_Directory = DT_DIR,
    VNT_CharDevice = DT_CHR,
    VNT_BlockDevice = DT_BLK,
    VNT_Pipe = DT_FIFO,
    VNT_Symlink = DT_LNK,
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

struct VNode : public lib::Shared {
    VNodeType __type;
    lib::Ref<SuperBlock> __super_block;
    uint32_t __mode;
    uint64_t __inode;

    // Regular
    virtual Result<ssize_t> read(void* buf, size_t size, off_t offset) noexcept { return SYS_EISDIR; }
    virtual Result<ssize_t> write(const void* buf, size_t size, off_t offset) noexcept { return SYS_EISDIR; }
    virtual Result<> truncate(off_t length) noexcept { return SYS_EISDIR; }

    // Directory
    virtual Result<lib::Ref<VNode>> lookup(std::string_view name) noexcept { return {}; }
    virtual Result<int> readdir(dirent* buf, size_t size, off_t* offset) noexcept { return SYS_ENOTDIR; }
    virtual Result<> mkdir(std::string_view name, uint32_t mode) noexcept { return SYS_ENOTDIR; }
    virtual Result<> create(std::string_view name, uint32_t mode) noexcept { return SYS_ENOTDIR; }
    virtual Result<> link(std::string_view name, lib::Ref<VNode> target) noexcept { return SYS_ENOTDIR; }
    virtual Result<> unlink(std::string_view name) noexcept { return SYS_ENOTDIR; }

    // General
    virtual Result<> stat(struct stat* buf) noexcept = 0;
    virtual Result<> ioctl(uint32_t req, uint32_t param) noexcept { return SYS_ENOTTY; }
    virtual Result<lib::Ref<VNodeFileObj>> open(lib::Ref<VNode> self, uint32_t mode) noexcept;
};

#pragma clang diagnostic pop

}  // namespace nyan::fs
