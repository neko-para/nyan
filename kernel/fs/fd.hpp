#pragma once

#include <bits/fcntl.h>

#include "../lib/result.hpp"
#include "../lib/shared.hpp"
#include "file.hpp"

namespace nyan::fs {

struct FdObj : public lib::Shared {
    lib::Ref<FileObj> __file;
    bool __close_on_exec{};

    FdObj(lib::Ref<FileObj> file) noexcept : __file(file) {}
    virtual ~FdObj();

    virtual Result<ssize_t> read(void* buf, size_t size) const noexcept;
    virtual Result<ssize_t> write(const void* buf, size_t size) const noexcept;
    virtual Result<> ioctl(uint32_t req, uint32_t param) const noexcept;
    virtual Result<off_t> seek(off_t offset, int whence) const noexcept;
};

}  // namespace nyan::fs
