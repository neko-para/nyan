#pragma once

#include <cstddef>
#include <new>

namespace nyan::lib {

template <typename T>
struct Lazy {
    alignas(alignof(T)) std::byte __storage[sizeof(T)];

    template <typename... Args>
    void construct(Args&&... args) {
        new (__storage) T(std::forward<Args>(args)...);
    }

    T* operator->() noexcept { return std::launder(reinterpret_cast<T*>(__storage)); }
    const T* operator->() const noexcept { return std::launder(reinterpret_cast<const T*>(__storage)); }
};

}  // namespace nyan::lib