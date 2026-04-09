#pragma once

#include <stdint.h>
#include <algorithm>

#include "../arch/io.hpp"

namespace nyan::allocator {

constexpr uint32_t frameBase = 0xC0400000;
constexpr uint32_t frameTop = 0xFFFFF000;

constexpr uint32_t frameCount = (frameTop - frameBase) >> 12;

struct FrameManager {
    constexpr static uint32_t bitmap_size = frameCount >> 5;

    uint32_t bitmap[bitmap_size];

    FrameManager() { std::fill_n(bitmap, bitmap_size, 0); }
    ~FrameManager() = default;
    FrameManager(const FrameManager&) = delete;
    FrameManager& operator=(const FrameManager&) = delete;

    // offset to virtual
    static uint32_t frameAt(uint32_t offset) noexcept { return frameBase + (offset << 12); }
    // virtual to offset
    static uint32_t frameFor(uint32_t addr) noexcept { return (addr - frameBase) >> 12; }

    uint32_t alloc() noexcept {
        for (size_t i = 0; i < bitmap_size; i++) {
            auto pos = std::countr_one(bitmap[i]);
            if (pos < 32) {
                bitmap[i] |= 1 << pos;
                return (i << 5) | pos;
            }
        }
        arch::kfatal("frame not enough!");
    }
    void free(uint32_t offset) noexcept { bitmap[offset >> 5] &= ~(1 << (offset & 31)); }
};

}  // namespace nyan::allocator
