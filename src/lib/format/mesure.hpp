#pragma once

#include "spec.hpp"

namespace nyan::lib::__format {

template <typename T>
size_t mesure(const T& val, const format_spec& spec) = delete;

template <target_string T>
size_t mesure(const T& val, const format_spec& spec) {
    if constexpr (same_as_any<T, const char*, char*>) {
        return std::min(std::char_traits<char>::length(val), spec.precision);
    } else if constexpr (same_as_any<T, string_view, string>) {
        return std::min(val.size(), spec.precision);
    } else if constexpr (is_char_array<T>) {
        size_t capa = char_array_size<T>;
        size_t len = std::find(val, val + capa, 0);
        return std::min(len, spec.precision);
    } else {
        return 0;
    }
}

template <target_numeric T>
size_t mesure(const T& val, const format_spec& spec) {
    if (spec.type == 'c') {
        return 1;
    } else if (spec.type == 's') {
        if constexpr (target_bool<T>) {
            return val ? 4 : 5;
        } else {
            return 0;
        }
    }

    using U = std::make_unsigned_t<T>;

    size_t len = 0;

    U uval;
    if constexpr (std::is_signed_v<T>) {
        if (val < 0) {
            uval = U{0} - U{val};
            len++;
        } else {
            uval = val;
            if (spec.sign != '-') {
                len++;
            }
        }
    } else {
        uval = val;
        if (spec.sign != '-') {
            len++;
        }
    }

    if (spec.type == 'd') {
        do {
            len++;
            uval /= 10;
        } while (uval);
    } else if (spec.type == 'b' || spec.type == 'B') {
        if (spec.sharp) {
            len += 2;
        }
        len += std::max(1, std::bit_width(uval));
    } else if (spec.type == 'o') {
        if (spec.sharp) {
            len += 1;
        }
        do {
            len++;
            uval >>= 3;
        } while (uval);
    } else if (spec.type == 'x' || spec.type == 'X' || spec.type == 'p' || spec.type == 'P') {
        if (spec.sharp || spec.type == 'p') {
            len += 2;
        }
        do {
            len++;
            uval >>= 4;
        } while (uval);
    }

    if (spec.zero) {
        len = std::max(len, spec.width);
    }

    return len;
}

template <target_pointer T>
size_t mesure(const T& val, const format_spec& spec) {
    return mesure(reinterpret_cast<uintptr_t>(val), spec);
}

}  // namespace nyan::lib::__format
