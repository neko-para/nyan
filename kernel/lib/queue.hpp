#pragma once

#include <array>

namespace nyan::lib {

template <typename T, size_t N>
    requires(N > 0 && ((N & (N - 1)) == 0))
struct RingQueue {
    constexpr static size_t Mask = N - 1;

    std::array<T, N> data;
    size_t head{};
    size_t tail{};

    size_t size() const noexcept { return tail - head; }
    bool empty() const noexcept { return head == tail; }
    bool full() const noexcept { return size() == N; }

    template <typename U>
    bool push(const U& item) noexcept {
        if (full()) {
            return false;
        }
        data[tail++ & Mask] = static_cast<T>(item);
        return true;
    }

    template <typename U>
    size_t pushSome(const U* items, size_t count) noexcept {
        size_t result = 0;
        while (result < count && !full()) {
            data[tail++ & Mask] = static_cast<T>(*items++);
            result++;
        }
        return result;
    }

    bool pop(T& item) noexcept {
        if (empty()) {
            return false;
        }
        item = data[head++ & Mask];
        return true;
    }

    size_t popSome(T* items, size_t count) noexcept {
        size_t result = 0;
        while (result < count && !empty()) {
            *items++ = data[head++ & Mask];
            result++;
        }
        return result;
    }
};

}  // namespace nyan::lib
