#pragma once

#include <errno.h>
#include <sys/types.h>

inline void* operator new(size_t, void* __p) noexcept {
    return __p;
}
inline void* operator new[](size_t, void* __p) noexcept {
    return __p;
}
inline void operator delete(void*, void*) noexcept {}
inline void operator delete[](void*, void*) noexcept {}

namespace nyan::user {

template <typename T>
static T wrapRet(T ret) {
    if (ret < 0) {
        errno = -ret;
        return -1;
    } else {
        return ret;
    }
}

template <typename T>
T min(T a, T b) {
    return a < b ? a : b;
}

template <typename T>
T max(T a, T b) {
    return a > b ? a : b;
}

}  // namespace nyan::user
