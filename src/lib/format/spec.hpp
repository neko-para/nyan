#pragma once

#include "concepts.hpp"
#include "context.hpp"

namespace nyan::lib::__format {

struct format_spec {
    char fill = ' ';
    char align = 0;  // unset

    char sign = '-';
    bool sharp = false;
    bool zero = false;

    uint16_t width = 0;
    uint16_t precision = uint16_t(-1);

    char type = 0;

    constexpr static bool is_align(char ch) { return ch == '<' || ch == '>' || ch == '^'; }
    constexpr static bool is_sign(char ch) { return ch == '+' || ch == '-' || ch == ' '; }
    constexpr static bool is_digit(char ch) { return ch >= '0' && ch <= '9'; }

    constexpr parse_context::iterator parse_fill_align(parse_context ctx) {
        auto it = ctx.begin();
        if (it == ctx.end()) {
            return ctx.begin();
        } else if (is_align(*it)) {
            align = *it++;
            return it;
        } else {
            auto next = it;
            next++;
            if (next == ctx.end()) {
                return ctx.begin();
            } else if (is_align(*next)) {
                align = *next++;
                fill = *it;
                return next;
            } else {
                return ctx.begin();
            }
        }
    }

    constexpr parse_context::iterator parse_sign_sharp_zero(parse_context ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && is_sign(*it)) {
            sign = *it++;
        }
        if (it != ctx.end() && *it == '#') {
            sharp = true;
            it++;
        }
        if (it != ctx.end() && *it == '0') {
            zero = (align == 0);
            it++;
        }
        return it;
    }

    constexpr parse_context::iterator parse_width_precision(parse_context ctx) {
        auto it = ctx.begin();
        while (it != ctx.end() && is_digit(*it)) {
            width *= 10;
            width += *it - '0';
            it++;
        }
        if (it != ctx.end() && *it == '.') {
            auto next = it;
            next++;
            if (next == ctx.end() || !is_digit(*next)) {
                return it;
            }
            it++;
            while (it != ctx.end() && is_digit(*it)) {
                precision *= 10;
                precision += *it - '0';
                it++;
            }
        }
        return it;
    }

    template <typename T>
    constexpr parse_context::iterator parse_type(parse_context ctx) {
        auto it = ctx.begin();
        if constexpr (target_string<T>) {
            if (it != ctx.end() && *it == 's') {
                type = *it++;
            } else {
                type = 's';
            }
        } else if constexpr (target_integral<T> || target_char<T>) {
            auto known = string_view{"cbBdoxX"};
            if (it != ctx.end() && known.find(*it) != string_view::npos) {
                type = *it++;
            } else {
                if constexpr (target_char<T>) {
                    type = 'c';
                } else {
                    type = 'd';
                }
            }
        } else if constexpr (target_bool<T>) {
            auto known = string_view{"bBdoxXs"};
            if (it != ctx.end() && known.find(*it) != string_view::npos) {
                type = *it++;
            } else {
                type = 's';
            }
        } else if constexpr (target_pointer<T>) {
            if (it != ctx.end() && (*it == 'p' || *it == 'P')) {
                type = *it++;
            } else {
                type = 'p';
            }
        }
        return it;
    }

    template <typename T>
    constexpr parse_context::iterator parse(parse_context ctx) {
        auto it = ctx.begin();

        it = parse_fill_align({it, ctx.end()});
        if constexpr (target_numeric<T> || target_pointer<T>) {
            it = parse_sign_sharp_zero({it, ctx.end()});
        }
        it = parse_width_precision({it, ctx.end()});
        it = parse_type<T>({it, ctx.end()});

        if (!align) {
            auto num_pres = string_view{"bBdoxXpP"};
            if (num_pres.find(type) != string_view::npos) {
                align = '>';
            } else {
                align = '<';
            }
        }

        return it;
    }
};

}  // namespace nyan::lib::__format
