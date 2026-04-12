#pragma once

#include <concepts>
#include <functional>
#include <iterator>

#include "../containers.hpp"

namespace nyan::lib::__format {

template <typename Iter>
concept output_iterator = std::output_iterator<Iter, const char&>;

template <std::integral T, output_iterator Iter>
Iter toChars(Iter ptr, T val) {
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

    char buf[std::numeric_limits<U>::digits10 + 2];
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

template <std::unsigned_integral T, bool upper_case = true, output_iterator Iter>
Iter toCharsHex(Iter ptr, T val) {
    constexpr size_t bits = sizeof(T) * 2;
    constexpr const char* map = upper_case ? "0123456789ABCDEF" : "0123456789abcdef";
    for (int i = bits - 1; i >= 0; i--) {
        auto v = (val >> (i * 4)) & 0xF;
        *ptr++ = map[v];
    }
    return ptr;
}

template <typename T, output_iterator Iter>
Iter toChars(Iter ptr, T* val) {
    return toCharsHex(ptr, reinterpret_cast<uint32_t>(val));
}

template <output_iterator Iter>
Iter toChars(Iter ptr, const char* val) {
    while (*val) {
        *ptr++ = *val++;
    }
    return ptr;
}

template <output_iterator Iter>
Iter toChars(Iter ptr, string_view val) {
    return std::copy(val.begin(), val.end(), ptr);
}

}  // namespace nyan::lib::__format
