#pragma once

#include <new>
#include <utility>

namespace nyan::allocator {

void* frameAlloc();
void frameFree(void* frame);

template <typename T, typename... Args>
inline T* frameAllocAs(Args&&... args) noexcept {
    return new (frameAlloc()) T(std::forward<Args>(args)...);
}

template <typename T>
inline void frameFreeAs(T* frame) noexcept {
    frame->~T();
    frameFree(frame);
}

}  // namespace nyan::allocator
