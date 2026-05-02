#pragma once

#include "map.hpp"

namespace nyan::allocator {

struct PoolManager {
    uint32_t* __bitmap;
    size_t __bitmap_size;

    PoolManager(uint32_t size) noexcept;
    ~PoolManager() = default;
    PoolManager(const PoolManager&) = delete;
    PoolManager& operator=(const PoolManager&) = delete;

    // offset to physical
    static uint32_t pageAt(uint32_t offset) noexcept { return __pool_base + (offset << 12); }
    // physical to offset
    static uint32_t pageFor(uint32_t addr) noexcept { return (addr - __pool_base) >> 12; }

    uint32_t alloc() noexcept;
    void free(uint32_t offset) noexcept;
};

}  // namespace nyan::allocator
