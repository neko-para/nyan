#pragma once

#include <sys/types.h>

#include "../lib/shared.hpp"

namespace nyan::fs {

struct FileObj : public lib::Shared {
    virtual ssize_t read(void* buf, size_t size) noexcept = 0;
    virtual ssize_t write(const void* buf, size_t size) noexcept = 0;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept = 0;

    virtual void onFdClose([[maybe_unused]] uint32_t mode) noexcept {}
};

}  // namespace nyan::fs
