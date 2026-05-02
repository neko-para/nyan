#pragma once

#include "../lib/list.hpp"
#include "../paging/address.hpp"

namespace nyan::allocator {

struct SlabChunk;
struct SlabHeader;
struct SlabCache;

struct SlabChunkTag {
    using type = SlabChunk;
};
struct SlabHeaderTag {
    using type = SlabHeader;
};

struct SlabChunk : public lib::ListNodes<SlabChunkTag> {
    static SlabChunk* fromAddr(void* addr) noexcept { return static_cast<SlabChunk*>(addr); }
    static SlabChunk* fromAddr(uint32_t addr) noexcept { return reinterpret_cast<SlabChunk*>(addr); }
};

struct SlabHeader : public lib::ListNodes<SlabHeaderTag> {
    lib::List<SlabChunkTag, false> __first_chunk;
    uint16_t __chunk_size;  // 1 << chunk_size
    uint16_t __used_count;
    SlabCache* __cache;

    SlabHeader(size_t size, SlabCache* cache);
    ~SlabHeader() = default;
    SlabHeader(const SlabHeader&) = delete;
    SlabHeader& operator=(const SlabHeader&) = delete;

    static SlabHeader* fromAddr(void* addr) noexcept {
        return paging::VirtualAddress{addr}.thisPage().as<SlabHeader>();
    }

    uint16_t fullCount() const noexcept { return (4096 >> __chunk_size) - 1; }
    bool full() const noexcept { return __used_count == fullCount(); }
    bool aboutToFull() const noexcept { return __used_count == fullCount() - 1; }
    bool empty() const noexcept { return __used_count == 0; }

    void* alloc() noexcept;
    void free(void* addr) noexcept;
};

struct SlabCache {
    lib::List<SlabHeaderTag, false> full_slabs;
    lib::List<SlabHeaderTag, false> used_slabs;
};

struct SlabManager {
    // 16, 32, 64, 128, 256, 512
    SlabCache caches[6];

    SlabManager() noexcept = default;
    ~SlabManager() = default;
    SlabManager(const SlabManager&) = delete;
    SlabManager& operator=(const SlabManager&) = delete;

    SlabCache* findSuitableCache(size_t size, size_t& chunk_size) noexcept;
    void* alloc(size_t size) noexcept;
    void free(void* addr) noexcept;
};

}  // namespace nyan::allocator
