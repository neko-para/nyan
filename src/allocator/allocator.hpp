#pragma once

#include <sys/types.h>

#include "utils.hpp"

namespace nyan::allocator {

template <typename T>
struct SlabAllocator {
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using is_always_equal = std::true_type;

    SlabAllocator() noexcept = default;
    template <typename U>
    SlabAllocator(const SlabAllocator<U>&) noexcept {}

    value_type* allocate(size_type n) const noexcept {
        return static_cast<value_type*>(alloc(n * sizeof(value_type), alignof(value_type)));
    }
    void deallocate(value_type* ptr, size_type) const noexcept { free(ptr); }

    bool operator==(const SlabAllocator<T>&) const { return true; }
    bool operator!=(const SlabAllocator<T>&) const { return false; }
};

}  // namespace nyan::allocator
