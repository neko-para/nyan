#pragma once

#include <stddef.h>
#include <stdint.h>
#include <bit>

#include "../arch/io.hpp"

namespace nyan::allocator {

struct SlabCache;

struct SlabChunk {
    SlabChunk* next_chunk;

    static SlabChunk* fromAddr(void* addr) noexcept { return static_cast<SlabChunk*>(addr); }
};

struct SlabHeader : public SlabChunk {
    uint16_t chunk_size;  // 1 << chunk_size
    uint16_t used_count;
    SlabHeader* next_slab;
    SlabHeader* prev_slab;
    SlabCache* cache;

    SlabHeader(size_t size, SlabCache* cache);
    ~SlabHeader() = default;
    SlabHeader(const SlabHeader&) = delete;
    SlabHeader& operator=(const SlabHeader&) = delete;

    void* alloc() noexcept;
    void free(void* addr) noexcept;
    bool full() const noexcept { return used_count == ((4096 >> chunk_size) - 1); }
    bool empty() const noexcept { return used_count == 0; }

    static SlabHeader* fromAddr(void* addr) noexcept {
        return reinterpret_cast<SlabHeader*>(reinterpret_cast<uint32_t>(addr) & (~0x3FF));
    }

    SlabHeader* pushAsFront(SlabHeader*& list) noexcept {
        prev_slab = nullptr;
        auto next = next_slab;
        if (next) {
            next->prev_slab = nullptr;
        }
        next_slab = list;
        if (next_slab) {
            next_slab->prev_slab = this;
        }
        list = this;
        return next;
    }
    SlabHeader* take(SlabHeader*& list) noexcept {
        if (next_slab) {
            next_slab->prev_slab = prev_slab;
        }
        if (prev_slab) {
            prev_slab->next_slab = next_slab;
        } else {
            list = next_slab;
        }
        return this;
    }
};

struct SlabCache {
    SlabHeader* full_slabs = 0;
    SlabHeader* used_slabs = 0;
};

struct SlabManager {
    // 16, 32, 64, 128, 256, 512
    SlabCache caches[6];

    SlabManager() = default;
    ~SlabManager() = default;
    SlabManager(const SlabManager&) = delete;
    SlabManager& operator=(const SlabManager&) = delete;

    SlabCache* findSuitableCache(size_t size, size_t& chunk_size) noexcept {
        if (size > 512) {
            arch::kfatalfmt("%zu too large", size);
        }
        if (size == 0) {
            arch::kfatal("alloc 0 size");
        }
        chunk_size = std::bit_width(size - 1) + 1;
        return &caches[chunk_size - 4];
    }

    void* alloc(size_t size) noexcept;
    void free(void* addr) noexcept;
};

extern SlabManager* slabManager;

}  // namespace nyan::allocator
