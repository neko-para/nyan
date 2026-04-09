#pragma once

#include <new>
#include <utility>

namespace nyan::allocator {

void* frameAlloc();
void frameFree(void* frame);
void* alloc(size_t size);
void free(void* addr);

template <typename T, typename... Args>
inline T* frameAllocAs(Args&&... args) noexcept {
    return new (frameAlloc()) T(std::forward<Args>(args)...);
}

template <typename T>
inline void frameFreeAs(T* frame) noexcept {
    frame->~T();
    frameFree(frame);
}

template <typename T, typename... Args>
inline T* allocAs(Args&&... args) noexcept {
    return new (alloc(sizeof(T))) T(std::forward<Args>(args)...);
}

template <typename T>
inline void freeAs(T* frame) noexcept {
    frame->~T();
    free(frame);
}

}  // namespace nyan::allocator
