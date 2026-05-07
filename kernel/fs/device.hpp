#pragma once

#include <sys/types.h>

#include "../lib/result.hpp"

namespace nyan::fs {

struct CharDevice {
    virtual ~CharDevice() = default;
    virtual Result<ssize_t> read(void* buf, size_t size) noexcept = 0;
    virtual Result<ssize_t> write(const void* buf, size_t size) noexcept = 0;
    virtual Result<> ioctl(uint32_t req, uint32_t param) noexcept = 0;
};

struct BlockDevice {
    virtual ~BlockDevice() = default;
    virtual Result<ssize_t> read(void* buf, size_t size, off_t offset) noexcept = 0;
    virtual Result<ssize_t> write(const void* buf, size_t size, off_t offset) noexcept = 0;
    virtual Result<> ioctl(uint32_t req, uint32_t param) noexcept = 0;
};

}  // namespace nyan::fs
