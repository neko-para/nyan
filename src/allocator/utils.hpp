#pragma once

#include <new>
#include <utility>

#include "../paging/address.hpp"

namespace nyan::allocator {

paging::PhysicalAddress physicalFrameAlloc();
void physicalFrameRetain(paging::PhysicalAddress addr);
void physicalFrameRelease(paging::PhysicalAddress addr);

paging::VirtualAddress virtualFrameAlloc();
void virtualFrameFree(paging::VirtualAddress addr);

void* frameAlloc();
void frameFree(void* frame);
void* slabAlloc(size_t size, size_t align);
void slabFree(void* addr);

template <typename T, typename... Args>
inline T* frameAllocAs(Args&&... args) noexcept {
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
    return new (slabAlloc(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
}

template <typename T>
inline void freeAs(T* frame) noexcept {
    if (!frame) {
        return;
    }
    frame->~T();
    slabFree(frame);
}

}  // namespace nyan::allocator
