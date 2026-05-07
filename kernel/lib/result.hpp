#pragma once

#include <concepts>
#include <type_traits>
#include <variant>

namespace nyan {

struct Errno {
    int __errno;

    constexpr explicit Errno(int error) noexcept : __errno(error) {}

    template <typename T>
        requires(sizeof(T) == 4)
    constexpr operator T() const noexcept {
        if constexpr (std::is_pointer_v<T>) {
            return reinterpret_cast<T>(-__errno);
        } else if constexpr (std::integral<T>) {
            return static_cast<T>(-__errno);
        } else {
            static_assert(false, "T is not pointer nor integral");
        }
    }
};

template <typename... T>
struct [[nodiscard]] Result;

template <typename T>
struct [[nodiscard]] Result<T> {
    union {
        T __value;
    };
    int __errno;

    Result() noexcept : __errno(0) {}
    Result(Errno err) noexcept : __errno(err.__errno) {}
    template <typename... Args>
        requires std::is_constructible_v<T, Args...>
    Result(Args&&... args) noexcept : __value(std::forward<Args>(args)...), __errno(0) {}
    ~Result() noexcept {
        if (__errno) {
            __value.~T();
        }
    }

    bool has_value() const noexcept { return __errno == 0; }
    operator bool() const noexcept { return has_value(); }

    const T& operator*() const& noexcept { return __value; }
    T& operator*() & noexcept { return __value; }
    T&& operator*() && noexcept { return std::move(__value); }

    const T& value() const& noexcept { return __value; }
    T& value() & noexcept { return __value; }
    T&& value() && noexcept { return std::move(__value); }

    Errno error() const noexcept { return Errno{__errno}; }

    static Result<T> extract(T value) noexcept
        requires(sizeof(T) == 4)
    {
        if constexpr (std::is_pointer_v<T>) {
            auto val = reinterpret_cast<int32_t>(value);
            if (val < 0) {
                return Errno{-val};
            } else {
                return value;
            }
        } else if constexpr (std::integral<T>) {
            auto val = static_cast<int32_t>(value);
            if (val < 0) {
                return Errno{-val};
            } else {
                return value;
            }
        } else {
            static_assert(false, "T is not pointer nor integral");
        }
    }

    T merge() const noexcept
        requires(sizeof(T) == 4)
    {
        if (__errno) {
            return Errno{__errno};
        } else {
            return __value;
        }
    }

    bool operator==(Errno error) const noexcept { return __errno == error.__errno; }
};

template <>
struct [[nodiscard]] Result<> {
    int __errno;

    Result() noexcept : __errno(0) {}
    Result(Errno err) noexcept : __errno(err.__errno) {}
    ~Result() noexcept = default;

    bool has_value() const noexcept { return __errno == 0; }
    operator bool() const noexcept { return has_value(); }

    auto operator*() const noexcept { return nullptr; }
    auto value() const noexcept { return nullptr; }

    Errno error() const noexcept { return Errno{__errno}; }

    static Result<> extract(int value) noexcept {
        if (value < 0) {
            return Errno{-value};
        } else {
            return {};
        }
    }

    int merge() const noexcept { return -__errno; }

    bool operator==(Errno error) const noexcept { return __errno == error.__errno; }
};

}  // namespace nyan

#define __try(expr)                  \
    ({                               \
        auto __result = (expr);      \
        if (!__result) {             \
            return __result.error(); \
        }                            \
        std::move(*__result);        \
    })
