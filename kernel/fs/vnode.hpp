#pragma once

#include <dirent.h>
#include <nyan/errno.h>
#include <sys/stat.h>
#include <sys/types.h>

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
    SuperBlock* __super_block;
    uint32_t __mode;
    uint64_t __inode;

    // Regular
    virtual ssize_t read(void* buf, size_t size, off_t offset) noexcept { return -SYS_EISDIR; }
    virtual ssize_t write(const void* buf, size_t size, off_t offset) noexcept { return -SYS_EISDIR; }
    virtual int truncate(off_t length) noexcept { return -SYS_EISDIR; }

    // Directory
    virtual lib::Ref<VNode> lookup(const char* name) noexcept { return {}; }
    virtual int readdir(dirent* buf, size_t size, off_t* offset) noexcept { return -SYS_ENOTDIR; }
    virtual int mkdir(const char* name, uint32_t mode) noexcept { return -SYS_ENOTDIR; }
    virtual int touch(const char* name, uint32_t mode) noexcept { return -SYS_ENOTDIR; }
    virtual int link(const char* name, lib::Ref<VNode> target) noexcept { return -SYS_ENOTDIR; }
    virtual int unlink(const char* name) noexcept { return -SYS_ENOTDIR; }

    // General
    virtual int stat(struct stat* buf) noexcept = 0;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept { return -SYS_ENOTTY; }
};

#pragma clang diagnostic pop

}  // namespace nyan::fs
