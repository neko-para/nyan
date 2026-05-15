#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../lib/result.hpp"
#include "../lib/shared.hpp"
#include "forward.hpp"

struct stat;

namespace nyan::fs {

struct FileObj : public lib::Shared {
    uint32_t __mode{};
    off_t __offset{};

    FileObj() = default;
    FileObj(uint32_t mode) : __mode(mode) {}

    virtual lib::Ref<VNode> getVNode() noexcept { return {}; }

    virtual Result<ssize_t> read(void* buf, size_t size) noexcept = 0;
    virtual Result<ssize_t> write(const void* buf, size_t size) noexcept = 0;
    virtual Result<> stat(struct stat* buf) noexcept = 0;
    virtual Result<> ioctl(unsigned req, uint32_t param) noexcept = 0;
    virtual Result<off_t> seek(off_t offset, int whence) noexcept = 0;

    virtual void onFdClose() noexcept {}
};

}  // namespace nyan::fs
