#pragma once

#include "../fs/device.hpp"
#include "forward.hpp"

namespace nyan::console {

struct TtyDevice : public fs::CharDevice {
    Tty* __tty;

    TtyDevice(Tty* tty) noexcept : __tty(tty) {}

    virtual ssize_t read(void* buf, size_t size) noexcept override;
    virtual ssize_t write(const void* buf, size_t size) noexcept override;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept override;
};

}  // namespace nyan::console