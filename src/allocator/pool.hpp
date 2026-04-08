#pragma once

#include <algorithm>
#include <bit>

#include "../arch/io.hpp"

namespace nyan::allocator {

constexpr uint32_t base = 0x400000;  // 4M

struct PoolManager {
    uint32_t* bitmap;
    size_t bitmap_size;

    PoolManager(uint32_t size) {
        bitmap_size = (size >> 12) >> 5;
        bitmap = new uint32_t[bitmap_size];
        std::fill_n(bitmap, bitmap_size, 0);

        printf("bitmap size %u\n", bitmap_size);
    }
    ~PoolManager() { delete[] bitmap; }

    PoolManager(const PoolManager&) = delete;
    PoolManager& operator=(const PoolManager&) = delete;

    // offset to physical
    static uint32_t pageAt(uint32_t offset) noexcept { return base + (offset << 12); }
    // physical to offset
    static uint32_t pageFor(uint32_t addr) noexcept { return (addr - base) >> 12; }

    uint32_t alloc() noexcept {
        for (size_t i = 0; i < bitmap_size; i++) {
            auto pos = std::countr_one(bitmap[i]);
            if (pos < 32) {
                bitmap[i] |= 1 << pos;
                return (i << 5) | pos;
            }
        }
        arch::kfatal("page not enough!");
    }
    void free(uint32_t offset) noexcept { bitmap[offset >> 5] &= ~(1 << (offset & 31)); }
};

extern PoolManager* manager;

}  // namespace nyan::allocator
