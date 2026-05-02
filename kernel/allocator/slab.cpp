#include "slab.hpp"

#include "../arch/print.hpp"
#include "mod.hpp"

namespace nyan::allocator {

SlabHeader::SlabHeader(size_t size, SlabCache* cache) : __chunk_size(size), __used_count(0), __cache(cache) {
    auto self = reinterpret_cast<uint32_t>(this);
    auto dlt = 1 << size;
    auto cnt = fullCount();

    auto addr = self + 4096 - dlt;
    while (cnt--) {
        __first_chunk.push_front(SlabChunk::fromAddr(addr));
        addr -= dlt;
    }
}

void* SlabHeader::alloc() noexcept {
    __used_count += 1;
    auto ret = __first_chunk.front();
    __first_chunk.pop_front();
    return ret;
}

void SlabHeader::free(void* addr) noexcept {
    auto chunk = static_cast<SlabChunk*>(addr);
    __used_count -= 1;
    __first_chunk.push_front(chunk);
}

SlabCache* SlabManager::findSuitableCache(size_t size, size_t& chunk_size) noexcept {
    if (size > 512) {
        arch::kfatal("{} too large", size);
    } else if (size == 0) {
        arch::kfatal("alloc 0 size");
    } else if (size < 16) {
        size = 16;
    }
    chunk_size = std::bit_width(size - 1) + 1;
    return &caches[chunk_size - 5];
}

void* SlabManager::alloc(size_t size) noexcept {
    if (size == 0) {
        size = 1;
    }
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
        auto frame = frameAlloc();
        auto slab = new (frame) SlabHeader(chunk_size, cache);
        auto ret = slab->alloc();
        cache->used_slabs.push_front(slab);
        memset(ret, 0, size);
        return ret;
    }
}

void SlabManager::free(void* addr) noexcept {
    auto chunk = SlabChunk::fromAddr(addr);
    auto slab = SlabHeader::fromAddr(addr);
    auto cache = slab->__cache;
    slab->free(chunk);
    if (slab->aboutToFull()) {
        cache->full_slabs.erase({slab});
        cache->used_slabs.push_front(slab);
    } else if (slab->empty()) {
        cache->used_slabs.erase({slab});
        slab->~SlabHeader();
        frameFree(slab);
    }
}

}  // namespace nyan::allocator
