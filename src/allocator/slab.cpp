#include "slab.hpp"

#include "utils.hpp"

namespace nyan::allocator {

SlabHeader::SlabHeader(size_t size, SlabCache* cache) : chunk_size(size), used_count(0), cache(cache) {
    auto self = reinterpret_cast<uint32_t>(this);
    auto dlt = 1 << size;
    auto cnt = (4096 >> size) - 2;

    auto addr = self + dlt;
    first_chunk.head = reinterpret_cast<SlabChunk*>(addr);
    while (cnt--) {
        auto nextAddr = addr + dlt;
        reinterpret_cast<SlabChunk*>(addr)->next = reinterpret_cast<SlabChunk*>(nextAddr);
        addr = nextAddr;
    }
    reinterpret_cast<SlabChunk*>(self + 4096 - dlt)->next = 0;
}

void* SlabHeader::alloc() noexcept {
    used_count += 1;
    return first_chunk.popFront();
}

void SlabHeader::free(void* addr) noexcept {
    auto chunk = static_cast<SlabChunk*>(addr);
    used_count -= 1;
    first_chunk.pushFront(chunk);
}

void* SlabManager::alloc(size_t size) noexcept {
    size_t chunk_size;
    auto cache = findSuitableCache(size, chunk_size);

    if (cache->used_slabs) {
        auto ret = cache->used_slabs->alloc();
        if (cache->used_slabs->full()) {
            auto slab = cache->used_slabs.popFront();
            cache->full_slabs.pushFront(slab);
        }
        memset(ret, 0, size);
        return ret;
    } else {
        auto slab = frameAllocAs<SlabHeader>(chunk_size, cache);
        auto ret = slab->alloc();
        cache->used_slabs.pushFront(slab);
        memset(ret, 0, size);
        return ret;
    }
}

void SlabManager::free(void* addr) noexcept {
    auto chunk = SlabChunk::fromAddr(addr);
    auto slab = SlabHeader::fromAddr(addr);
    auto cache = slab->cache;
    slab->first_chunk.pushFront(chunk);

    slab->used_count -= 1;
    if (slab->aboutToFull()) {
        cache->full_slabs.take(slab);
        cache->used_slabs.pushFront(slab);
    } else if (slab->empty()) {
        cache->used_slabs.take(slab);
        frameFreeAs(slab);
    }
}

}  // namespace nyan::allocator
