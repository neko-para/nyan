#pragma once

#include <utility>

#include "../paging/address.hpp"
#include "map.hpp"

namespace nyan::allocator {

paging::PhysicalAddress physicalFrameAlloc();
void physicalFrameRetain(paging::PhysicalAddress addr);
void physicalFrameRelease(paging::PhysicalAddress addr);
uint16_t physicalFrameGetRef(paging::PhysicalAddress addr);

paging::VirtualAddress virtualFrameAlloc();
void virtualFrameFree(paging::VirtualAddress addr);

void* frameAlloc();
void frameFree(void* frame);
void* slabAlloc(size_t size, size_t align);
void slabFree(void* addr);
void* largeFrameAlloc(size_t page);
void largeFrameFree(void* frame);

inline void* autoAlloc(size_t size) noexcept {
    if (size <= 512) {
        return slabAlloc(size, size);
    } else if (size <= 4096) {
        return frameAlloc();
    } else {
        return largeFrameAlloc((size + 0xFFF) >> 12);
    }
}

inline void autoFree(void* frame) noexcept {
    if (!frame) {
        return;
    }
    auto addr = reinterpret_cast<uint32_t>(frame);

    if (addr >= largeFrameBase) {
        largeFrameFree(frame);
    } else if (addr & 0xFFF) {
        slabFree(frame);
    } else {
        frameFree(frame);
    }
}

template <typename T, typename... Args>
inline T* frameAllocAs(Args&&... args) noexcept {
    static_assert(sizeof(T) <= 4096);
    return new (frameAlloc()) T(std::forward<Args>(args)...);
}

template <typename T>
inline void frameFreeAs(T* frame) noexcept {
    if (!frame) {
        return;
    }
    frame->~T();
    frameFree(frame);
}

template <typename T, typename... Args>
inline T* allocAs(Args&&... args) noexcept {
    if constexpr (sizeof(T) <= 512) {
        return new (slabAlloc(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
    } else if constexpr (sizeof(T) <= 4096) {
        return new (frameAlloc()) T(std::forward<Args>(args)...);
    } else {
        constexpr size_t page = (sizeof(T) + 0xFFF) >> 12;
        return new (largeFrameAlloc(page)) T(std::forward<Args>(args)...);
    }
}

template <typename T>
inline void freeAs(T* frame) noexcept {
    if (!frame) {
        return;
    }
    frame->~T();
    if constexpr (sizeof(T) <= 512) {
        slabFree(frame);
    } else if constexpr (sizeof(T) <= 4096) {
        frameFree(frame);
    } else {
        largeFrameFree(frame);
    }
}

}  // namespace nyan::allocator
