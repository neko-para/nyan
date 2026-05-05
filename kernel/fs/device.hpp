#pragma once

#include <sys/types.h>

namespace nyan::fs {

struct CharDevice {
    virtual ~CharDevice() = default;
    virtual ssize_t read(void* buf, size_t size) noexcept = 0;
    virtual ssize_t write(const void* buf, size_t size) noexcept = 0;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept = 0;
};

struct BlockDevice {
    virtual ~BlockDevice() = default;
    virtual ssize_t read(void* buf, size_t size, off_t offset) noexcept = 0;
    virtual ssize_t write(const void* buf, size_t size, off_t offset) noexcept = 0;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept = 0;
};

}  // namespace nyan::fs
