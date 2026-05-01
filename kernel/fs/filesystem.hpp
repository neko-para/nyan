#pragma once

#include <tuple>

#include "../lib/shared.hpp"
#include "forward.hpp"

namespace nyan::fs {

struct FileSystem {
    virtual ~FileSystem() = default;

    virtual std::tuple<lib::Ref<SuperBlock>, lib::Ref<VNode>> mount(Device* device, const char* options) noexcept = 0;
};

}  // namespace nyan::fs
