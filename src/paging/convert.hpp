#pragma once

#include <stdint.h>

namespace nyan::paging {

inline uint32_t truncateAddr(void* ptr) {
    return reinterpret_cast<uint32_t>(ptr) & (~0xFFF);
}

template <typename T>
inline T* physicalToVirtual(T* ptr) {
    return reinterpret_cast<T*>(reinterpret_cast<uint32_t>(ptr) + 0xC0000000);
}

inline uint32_t physicalToVirtual(uint32_t ptr) {
    return ptr + 0xC0000000;
}

template <typename T>
inline T* virtualToPhysical(T* ptr) {
    return reinterpret_cast<T*>(reinterpret_cast<uint32_t>(ptr) - 0xC0000000);
}

inline uint32_t virtualToPhysical(uint32_t ptr) {
    return ptr - 0xC0000000;
}

}  // namespace nyan::paging
