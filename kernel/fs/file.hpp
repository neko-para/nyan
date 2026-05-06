#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../lib/result.hpp"
#include "../lib/shared.hpp"

namespace nyan::fs {

struct FileObj : public lib::Shared {
    uint32_t __mode{};
    off_t __offset{};

    FileObj() = default;
    FileObj(uint32_t mode) : __mode(mode) {}

    virtual Result<ssize_t> read(void* buf, size_t size) noexcept = 0;
    virtual Result<ssize_t> write(const void* buf, size_t size) noexcept = 0;
    virtual Result<int> ioctl(uint32_t req, uint32_t param) noexcept = 0;

    virtual void onFdClose() noexcept {}
};

}  // namespace nyan::fs
