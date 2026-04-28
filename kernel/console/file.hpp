#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../fs/file.hpp"
#include "forward.hpp"

namespace nyan::console {

struct TtyObj : public fs::FileObj {
    Tty* tty;

    TtyObj(Tty* tty) : tty(tty) {}

    virtual ssize_t read(void* buf, size_t size) noexcept override;
    virtual ssize_t write(const void* buf, size_t size) noexcept override;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept override;
};

}  // namespace nyan::console
