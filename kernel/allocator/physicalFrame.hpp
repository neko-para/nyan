#pragma once

#include <algorithm>

namespace nyan::allocator {

struct PhysicalFrame {
    uint16_t ref;
};

struct PhysicalFrameManager {
    PhysicalFrame* info;

    PhysicalFrameManager(size_t size) {
        info = new PhysicalFrame[size >> 12];
        std::fill_n(info, size >> 12, PhysicalFrame{});
    }
};

}  // namespace nyan::allocator
