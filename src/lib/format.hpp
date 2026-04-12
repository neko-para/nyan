#pragma once

#include <concepts>

#include "containers.hpp"

namespace nyan::lib {

template <std::integral T>
char* toChars(char* ptr, T val) {
    using U = std::make_unsigned_t<T>;

    U uval;
    if constexpr (std::is_signed_v<T>) {
        if (val < 0) {
            uval = U{0} - U{val};
            *ptr++ = '-';
        } else {
            uval = val;
        }
    } else {
        uval = val;
    }

    char buf[22];
    char* top = buf;
    do {
        *top++ = uval % 10 + '0';
        uval /= 10;
    } while (uval);
    while (top != buf) {
        *ptr++ = *--top;
    }
    return ptr;
}

template <std::unsigned_integral T, bool upper_case = true>
char* toCharsHex(char* ptr, T val) {
    constexpr size_t bits = sizeof(T) * 2;
    constexpr const char* map = upper_case ? "0123456789ABCDEF" : "0123456789abcdef";
    for (int i = bits - 1; i >= 0; i--) {
        auto v = (val >> (i * 4)) & 0xF;
        *ptr++ = map[v];
    }
    return ptr;
}

template <typename... Args>
struct format_string {
    string_view fmt;

    template <typename... SvArgs>
        requires std::is_constructible_v<string_view, SvArgs...>
    consteval format_string(SvArgs&&... args) : fmt(std::forward<SvArgs>(args)...) {
        check();
    }

    void check() {}
};

}  // namespace nyan::lib
