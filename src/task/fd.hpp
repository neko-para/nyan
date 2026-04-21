#pragma once

#include <bits/fcntl.h>

#include "../console/entry.hpp"
#include "../lib/shared.hpp"

namespace nyan::task {

struct FdObj : public lib::Shared {
    uint32_t mode;

    FdObj(uint32_t mode) : mode(mode) {}

    virtual ssize_t read(void* buf, size_t size) const noexcept = 0;
    virtual ssize_t write(const void* buf, size_t size) const noexcept = 0;
    virtual int ioctl(uint32_t req, uint32_t param) const noexcept = 0;
};

struct DebugConObj : public FdObj {
    DebugConObj() : FdObj(O_WRONLY) {}

    virtual ssize_t read(void* buf, size_t size) const noexcept override;
    virtual ssize_t write(const void* buf, size_t size) const noexcept override;
    virtual int ioctl(uint32_t req, uint32_t param) const noexcept override;
};

struct TtyObj : public FdObj {
    console::Tty* tty;

    TtyObj(console::Tty* tty, uint32_t mode) noexcept : FdObj(mode), tty(tty) {}

    virtual ssize_t read(void* buf, size_t size) const noexcept override;
    virtual ssize_t write(const void* buf, size_t size) const noexcept override;
    virtual int ioctl(uint32_t req, uint32_t param) const noexcept override;
};

}  // namespace nyan::task
