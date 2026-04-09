#pragma once

#include <stdint.h>
#include <algorithm>

#include "../arch/io.hpp"

namespace nyan::allocator {

constexpr uint32_t frameBase = 0xC0400000;
constexpr uint32_t frameTop = 0xFFFFF000;

constexpr uint32_t frameCount = (frameTop - frameBase) >> 12;

struct PageManager {
    constexpr static uint32_t bitmap_size = frameCount >> 5;

    uint32_t bitmap[bitmap_size];

    PageManager() { std::fill_n(bitmap, bitmap_size, 0); }
    ~PageManager() = default;
    PageManager(const PageManager&) = delete;
    PageManager& operator=(const PageManager&) = delete;

    // offset to virtual
    static uint32_t pageAt(uint32_t offset) noexcept { return frameBase + (offset << 12); }
    // virtual to offset
    static uint32_t pageFor(uint32_t addr) noexcept { return (addr - frameBase) >> 12; }

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

extern PageManager* pageManager;

}  // namespace nyan::allocator
