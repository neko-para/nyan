#pragma once

#include "../concepts.hpp"
#include "spec.hpp"

namespace nyan::lib::__format {

template <output_iterator Iter, typename T>
Iter emit(Iter out, const T& val, const format_spec& spec, size_t mesure_width) = delete;

template <output_iterator Iter, target_string T>
Iter emit(Iter out, const T& val, const format_spec&, size_t mesure_width) {
    if constexpr (same_as_any<T, const char*, char*>) {
        return std::copy(val, val + mesure_width, out);
    } else if constexpr (same_as_any<T, string_view, string>) {
        return std::copy(val.begin(), val.begin() + mesure_width, out);
    } else if constexpr (is_char_array<T>) {
        return std::copy(val, val + mesure_width, out);
    } else {
        return out;
    }
}

template <output_iterator Iter, target_numeric T>
Iter emit(Iter out, const T& val, const format_spec& spec, size_t) {
    if (spec.type == 'c') {
        *out++ = static_cast<char>(val);
        return out;
    } else if (spec.type == 's') {
        if constexpr (target_bool<T>) {
            if (val) {
                return std::copy_n("true", 4, out);
            } else {
                return std::copy_n("false", 5, out);
            }
        } else {
            return out;
        }
    }

    using U = std::make_unsigned_t<T>;

    size_t len = 0;

    U uval;
    if constexpr (std::is_signed_v<T>) {
        if (val < 0) {
            uval = U{0} - static_cast<U>(val);
            len++;
            *out++ = '-';
        } else {
            uval = val;
            if (spec.sign == '+') {
                *out++ = '+';
                len++;
            } else if (spec.sign == ' ') {
                *out++ = ' ';
                len++;
            }
        }
    } else {
        uval = val;
        if (spec.sign == '+') {
            *out++ = '+';
            len++;
        } else if (spec.sign == ' ') {
            *out++ = ' ';
            len++;
        }
    }

    if (spec.type == 'd') {
        char buf[std::numeric_limits<U>::digits10 + 2];
        char* ptr = buf;
        do {
            len++;
            *ptr++ = (uval % 10) + '0';
            uval /= 10;
        } while (uval);
        if (spec.zero) {
            while (len < spec.width) {
                *out++ = '0';
                len++;
            }
        }
        while (ptr != buf) {
            *out++ = *--ptr;
        }
    } else if (spec.type == 'b' || spec.type == 'B') {
        if (spec.sharp) {
            out = std::copy_n(spec.type == 'b' ? "0b" : "0B", 2, out);
            len += 2;
        }
        int bit_width = std::max(1, std::bit_width(uval));
        len += bit_width;
        if (spec.zero) {
            while (len < spec.width) {
                *out++ = '0';
                len++;
            }
        }
        for (int i = bit_width - 1; i >= 0; i--) {
            *out++ = (val & (1 << i)) ? '1' : '0';
        }
    } else if (spec.type == 'o') {
        if (spec.sharp) {
            *out++ = '0';
            len += 1;
        }
        char buf[sizeof(T) * 8 / 3 + 2];
        char* ptr = buf;
        do {
            len++;
            *ptr++ = (uval & 7) + '0';
            uval >>= 3;
        } while (uval);
        if (spec.zero) {
            while (len < spec.width) {
                *out++ = '0';
                len++;
            }
        }
        while (ptr != buf) {
            *out++ = *--ptr;
        }
    } else if (spec.type == 'x' || spec.type == 'X' || spec.type == 'p' || spec.type == 'P') {
        bool is_upper = spec.type == 'X' || spec.type == 'P';
        if (spec.sharp || spec.type == 'p' || spec.type == 'P') {
            out = std::copy_n(is_upper ? "0X" : "0x", 2, out);
            len += 2;
        }
        char buf[sizeof(T) * 2 + 2];
        char* ptr = buf;
        const char* map = is_upper ? "0123456789ABCDEF" : "0123456789abcdef";
        do {
            len++;
            *ptr++ = map[uval & 0xF];
            uval >>= 4;
        } while (uval);
        if (spec.zero) {
            while (len < spec.width) {
                *out++ = '0';
                len++;
            }
        }
        while (ptr != buf) {
            *out++ = *--ptr;
        }
    }

    return out;
}

template <output_iterator Iter, target_pointer T>
Iter emit(Iter out, const T& val, const format_spec& spec, size_t mesure_width) {
    return emit(out, reinterpret_cast<uintptr_t>(val), spec, mesure_width);
}

}  // namespace nyan::lib::__format
