#pragma once

#include <stddef.h>
#include <stdint.h>
#include <bit>

#include "../arch/io.hpp"
#include "../lib/list.hpp"

namespace nyan::allocator {

struct SlabChunk;
struct SlabHeader;
struct SlabCache;

struct SlabChunkTag {
    using type = SlabChunk;
};
struct SlabHeaderTag {
    using type = SlabHeader;
    constexpr static bool bidi = true;
};

struct SlabChunk : public lib::ListBase<SlabChunkTag> {
    static SlabChunk* fromAddr(void* addr) noexcept { return static_cast<SlabChunk*>(addr); }
};

struct SlabHeader : public lib::ListBase<SlabHeaderTag> {
    lib::List<SlabChunk> first_chunk;
    uint16_t chunk_size;  // 1 << chunk_size
    uint16_t used_count;
    SlabCache* cache;

    SlabHeader(size_t size, SlabCache* cache);
    ~SlabHeader() = default;
    SlabHeader(const SlabHeader&) = delete;
    SlabHeader& operator=(const SlabHeader&) = delete;

    void* alloc() noexcept;
    void free(void* addr) noexcept;
    bool full() const noexcept { return used_count == ((4096 >> chunk_size) - 1); }
    bool aboutToFull() const noexcept { return used_count == ((4096 >> chunk_size) - 2); }
    bool empty() const noexcept { return used_count == 0; }

    static SlabHeader* fromAddr(void* addr) noexcept {
        return reinterpret_cast<SlabHeader*>(reinterpret_cast<uint32_t>(addr) & (~0x3FF));
    }
};

struct SlabCache {
    lib::List<SlabHeader> full_slabs;
    lib::List<SlabHeader> used_slabs;
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
        } else if (size == 0) {
            arch::kfatal("alloc 0 size");
        } else if (size < 16) {
            size = 16;
        }
        chunk_size = std::bit_width(size - 1) + 1;
        return &caches[chunk_size - 4];
    }

    void* alloc(size_t size) noexcept;
    void free(void* addr) noexcept;
};

extern SlabManager* slabManager;

}  // namespace nyan::allocator
