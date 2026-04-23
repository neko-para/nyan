#pragma once

#include "../fs/file.hpp"

namespace nyan::arch {

struct DebugConObj : public fs::FileObj {
    virtual ssize_t read(void* buf, size_t size) noexcept override;
    virtual ssize_t write(const void* buf, size_t size) noexcept override;
    virtual int ioctl(uint32_t req, uint32_t param) noexcept override;
};

}  // namespace nyan::arch
