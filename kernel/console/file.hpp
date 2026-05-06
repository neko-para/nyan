#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../fs/file.hpp"
#include "forward.hpp"

namespace nyan::console {

struct TtyObj : public fs::FileObj {
    Tty* __tty;

    TtyObj(uint32_t mode, Tty* tty) noexcept : FileObj(mode), __tty(tty) {}

    virtual Result<ssize_t> read(void* buf, size_t size) noexcept override;
    virtual Result<ssize_t> write(const void* buf, size_t size) noexcept override;
    virtual Result<int> ioctl(uint32_t req, uint32_t param) noexcept override;
};

}  // namespace nyan::console
