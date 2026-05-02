#include "frame.hpp"

#include <algorithm>
#include <bit>

#include "../arch/print.hpp"

namespace nyan::allocator {

FrameManager::FrameManager() noexcept {
    std::fill_n(__bitmap, __bitmap_size, 0);
}

uint32_t FrameManager::alloc() noexcept {
    for (size_t i = 0; i < __bitmap_size; i++) {
        auto pos = std::countr_one(__bitmap[i]);
        if (pos < 32) {
            __bitmap[i] |= 1 << pos;
            return (i << 5) | pos;
        }
    }
    arch::kfatal("frame not enough!");
}

void FrameManager::free(uint32_t offset) noexcept {
    __bitmap[offset >> 5] &= ~(1 << (offset & 31));
}

}  // namespace nyan::allocator