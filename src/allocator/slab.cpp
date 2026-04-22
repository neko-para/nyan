#include "slab.hpp"

#include "utils.hpp"

namespace nyan::allocator {

SlabHeader::SlabHeader(size_t size, SlabCache* cache) : chunk_size(size), used_count(0), cache(cache) {
    auto self = reinterpret_cast<uint32_t>(this);
    auto dlt = 1 << size;
    auto cnt = (4096 >> size) - 1;

    auto addr = self + 4096 - dlt;
    while (cnt--) {
        first_chunk.push_front(reinterpret_cast<SlabChunk*>(addr));
        addr -= dlt;
    }
}

void* SlabHeader::alloc() noexcept {
    used_count += 1;
    auto ret = first_chunk.front();
    first_chunk.pop_front();
    return ret;
}

void SlabHeader::free(void* addr) noexcept {
    auto chunk = static_cast<SlabChunk*>(addr);
    used_count -= 1;
    first_chunk.push_front(chunk);
}

void* SlabManager::alloc(size_t size) noexcept {
    size_t chunk_size;
    auto cache = findSuitableCache(size, chunk_size);

    if (!cache->used_slabs.empty()) {
        auto ret = cache->used_slabs.front()->alloc();
        if (cache->used_slabs.front()->full()) {
            auto slab = cache->used_slabs.front();
            cache->used_slabs.pop_front();
            cache->full_slabs.push_front(slab);
        }
        memset(ret, 0, size);
        return ret;
    } else {
        auto slab = frameAllocAs<SlabHeader>(chunk_size, cache);
        auto ret = slab->alloc();
        cache->used_slabs.push_front(slab);
        memset(ret, 0, size);
        return ret;
    }
}

void SlabManager::free(void* addr) noexcept {
    auto chunk = SlabChunk::fromAddr(addr);
    auto slab = SlabHeader::fromAddr(addr);
    auto cache = slab->cache;
    slab->free(chunk);
    if (slab->aboutToFull()) {
        cache->full_slabs.erase({slab});
        cache->used_slabs.push_front(slab);
    } else if (slab->empty()) {
        cache->used_slabs.erase({slab});
        frameFreeAs(slab);
    }
}

}  // namespace nyan::allocator
