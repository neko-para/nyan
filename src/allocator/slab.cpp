#include "slab.hpp"

#include "utils.hpp"

namespace nyan::allocator {

SlabHeader::SlabHeader(size_t size, SlabCache* cache)
    : chunk_size(size), used_count(0), next_slab(0), prev_slab(0), cache(cache) {
    auto self = reinterpret_cast<uint32_t>(this);
    auto upper = self + 4096;
    auto dlt = 1 << size;

    // first_free顺便也填上
    for (auto addr = self + dlt; addr != upper; addr += dlt) {
        reinterpret_cast<SlabChunk*>(addr - dlt)->next_chunk = reinterpret_cast<SlabChunk*>(addr);
    }
    reinterpret_cast<SlabChunk*>(upper - dlt)->next_chunk = 0;
}

void* SlabHeader::alloc() noexcept {
    auto chunk = next_chunk;
    next_chunk = chunk->next_chunk;
    used_count += 1;
    return chunk;
}

void SlabHeader::free(void* addr) noexcept {
    auto chunk = static_cast<SlabChunk*>(addr);
    chunk->next_chunk = next_chunk;
    next_chunk = chunk;
    used_count = 0;
}

void* SlabManager::alloc(size_t size) noexcept {
    size_t chunk_size;
    auto cache = findSuitableCache(size, chunk_size);

    if (cache->used_slabs) {
        auto ret = cache->used_slabs->alloc();
        if (cache->used_slabs->full()) {
            cache->used_slabs->take(cache->used_slabs)->pushAsFront(cache->full_slabs);
        }
        return ret;
    } else {
        auto slab = frameAllocAs<SlabHeader>(chunk_size, cache);
        auto ret = slab->alloc();
        slab->pushAsFront(cache->used_slabs);
        return ret;
    }
}

void SlabManager::free(void* addr) noexcept {
    auto chunk = SlabChunk::fromAddr(addr);
    auto slab = SlabHeader::fromAddr(addr);
    auto cache = slab->cache;
    chunk->next_chunk = slab->next_chunk;
    slab->next_chunk = chunk;

    if (slab->full()) {
        slab->used_count -= 1;
        slab->take(cache->full_slabs)->pushAsFront(cache->used_slabs);
    } else {
        slab->used_count -= 1;
        if (slab->empty()) {
            slab->take(cache->used_slabs);
            frameFreeAs(slab);
        }
    }
}

}  // namespace nyan::allocator
