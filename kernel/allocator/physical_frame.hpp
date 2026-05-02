#pragma once

#include <algorithm>

namespace nyan::allocator {

struct PhysicalFrame {
    uint16_t __ref;
};

struct PhysicalFrameManager {
    PhysicalFrame* __info;

    PhysicalFrameManager(size_t size) noexcept;
};

}  // namespace nyan::allocator
