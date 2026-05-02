#pragma once

#include "map.hpp"

namespace nyan::allocator {

struct FrameManager {
    constexpr static uint32_t __frame_count = (__frame_top - __frame_base) >> 12;
    constexpr static uint32_t __bitmap_size = __frame_count >> 5;

    uint32_t __bitmap[__bitmap_size];

    FrameManager() noexcept;
    ~FrameManager() = default;
    FrameManager(const FrameManager&) = delete;
    FrameManager& operator=(const FrameManager&) = delete;

    // offset to virtual
    static uint32_t frameAt(uint32_t offset) noexcept { return __frame_base + (offset << 12); }
    // virtual to offset
    static uint32_t frameFor(uint32_t addr) noexcept { return (addr - __frame_base) >> 12; }

    uint32_t alloc() noexcept;
    void free(uint32_t offset) noexcept;
};

}  // namespace nyan::allocator
