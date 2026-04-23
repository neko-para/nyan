#pragma once

#include <bits/fcntl.h>

#include "../lib/shared.hpp"
#include "file.hpp"

namespace nyan::fs {

struct FdObj : public lib::Shared {
    lib::Ref<FileObj> __file;
    uint32_t __mode;

    FdObj(lib::Ref<FileObj> file, uint32_t mode) : __file(file), __mode(mode) {}

    virtual ssize_t read(void* buf, size_t size) const noexcept;
    virtual ssize_t write(const void* buf, size_t size) const noexcept;
    virtual int ioctl(uint32_t req, uint32_t param) const noexcept;
};

}  // namespace nyan::fs
