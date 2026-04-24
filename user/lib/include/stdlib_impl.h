#pragma once

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

namespace nyan::user {

inline void* __freePages[16];

inline void* __allocPage() {
    for (auto& page : __freePages) {
        if (page) {
            auto result = page;
            page = nullptr;
            return result;
        }
    }
    // 内核确保首次就是对齐的; 之后直接换mmap
    return sbrk(4096);
}

inline void __freePage(void* page) {
    for (auto& slot : __freePages) {
        if (!slot) {
            slot = page;
            break;
        }
    }
}

inline unsigned bit_width(unsigned x) {
    if (x == 0)
        return 0;
    return 32 - __builtin_clz(x);
}

struct SlabCache;

struct SlabChunk {
    SlabChunk* __next{};

    static SlabChunk* fromAddr(void* addr) noexcept { return static_cast<SlabChunk*>(addr); }
};

struct SlabHeader {
    SlabHeader* __next{};
    SlabChunk* __first_chunk{};
    uint16_t __chunk_size;  // 1 << chunk_size
    uint16_t __used_count;
    SlabCache* __cache;

    SlabHeader(size_t size, SlabCache* cache);
    ~SlabHeader() = default;
    SlabHeader(const SlabHeader&) = delete;
    SlabHeader& operator=(const SlabHeader&) = delete;

    void* alloc() noexcept;
    void free(void* addr) noexcept;
    bool full() const noexcept { return __used_count == ((4096 >> __chunk_size) - 1); }
    bool aboutToFull() const noexcept { return __used_count == ((4096 >> __chunk_size) - 2); }
    bool empty() const noexcept { return __used_count == 0; }

    static SlabHeader* fromAddr(void* addr) noexcept {
        return reinterpret_cast<SlabHeader*>(reinterpret_cast<uint32_t>(addr) & (~0xFFF));
    }
};

struct SlabCache {
    SlabHeader* __full_slabs{};
    SlabHeader* __used_slabs{};
};

struct SlabManager {
    SlabCache __caches[6];

    SlabCache* findSuitableCache(size_t size, size_t& chunk_size) noexcept {
        if (size > 512) {
            return nullptr;
        } else if (size == 0) {
            return nullptr;
        } else if (size < 16) {
            size = 16;
        }
        chunk_size = bit_width(size - 1) + 1;
        return &__caches[chunk_size - 5];
    }

    size_t allocSize(void* addr) const noexcept {
        auto slab = SlabHeader::fromAddr(addr);
        return 1u << slab->__chunk_size;
    }

    void* alloc(size_t size) noexcept;
    void free(void* addr) noexcept;
};

}  // namespace nyan::user
