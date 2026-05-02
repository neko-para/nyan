#pragma once

#include "../lib/shared.hpp"
#include "forward.hpp"

namespace nyan::fs {

struct FileSystem {
    virtual ~FileSystem() = default;

    virtual lib::Ref<MountEntry> mount(Device* device, const char* options) noexcept = 0;
};

}  // namespace nyan::fs
