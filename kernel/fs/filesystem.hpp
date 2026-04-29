#pragma once

#include "forward.hpp"

namespace nyan::fs {

struct FileSystem {
    virtual ~FileSystem() = default;

    virtual SuperBlock* mount(Device* device, const char* options) noexcept = 0;
};

}  // namespace nyan::fs
