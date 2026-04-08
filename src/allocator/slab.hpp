#pragma once

#include <stddef.h>
#include <stdint.h>

namespace nyan::allocator {

constexpr size_t SlabPage = 1 << 12;  // 4K

struct SlabHeader;

struct SlabCache {
    uint32_t chunk_size;
    SlabHeader* full_slabs;
    SlabHeader* used_slabs;
    SlabHeader* empty_slabs;
};

struct alignas(SlabPage) SlabHeader {
    SlabCache* parent_cache;
    uint32_t used_count;
    void* first_free;
    SlabHeader* next_slab;
};

}  // namespace nyan::allocator
