#include "pool.hpp"

#include <algorithm>
#include <bit>

#include "../arch/print.hpp"

namespace nyan::allocator {

static uint32_t __pool_bitmap_storage[1 << 15];

PoolManager::PoolManager(uint32_t size) noexcept {
    __bitmap_size = (size >> 12) >> 5;
    __bitmap = __pool_bitmap_storage;
    std::fill_n(__bitmap, __bitmap_size, 0);
}

uint32_t PoolManager::alloc() noexcept {
    for (size_t i = 0; i < __bitmap_size; i++) {
        auto pos = std::countr_one(__bitmap[i]);
        if (pos < 32) {
            __bitmap[i] |= 1 << pos;
            return (i << 5) | pos;
        }
    }
    arch::kfatal("page not enough!");
}
void PoolManager::free(uint32_t offset) noexcept {
    __bitmap[offset >> 5] &= ~(1 << (offset & 31));
}

}  // namespace nyan::allocator