#pragma once

#include "concepts.hpp"
#include "context.hpp"
#include "emit.hpp"
#include "mesure.hpp"
#include "spec.hpp"

namespace nyan::lib::__format {

template <typename T, typename Iter>
struct formatter {
    // constexpr parse_context::iterator parse(parse_context ctx) = delete;

    // template <typename Context>
    // Context::iterator format(const T& val, Context& ctx) const = delete;
};

template <known_target T, typename Iter>
struct formatter<T, Iter> {
    format_spec spec;

    constexpr parse_context::iterator parse(parse_context ctx) { return spec.parse<T>(ctx); }

    template <typename Context>
    Context::iterator format(const T& val, Context& ctx) const {
        size_t mesure_width = mesure(val, spec);
        if (spec.width > mesure_width) {
            size_t padding = spec.width - mesure_width;
            auto it = ctx.iter;
            if (spec.align == '<') {
                it = emit(it, val, spec, mesure_width);
                for (size_t i = 0; i < padding; i++) {
                    *it++ = spec.fill;
                }
            } else if (spec.align == '>') {
                for (size_t i = 0; i < padding; i++) {
                    *it++ = spec.fill;
                }
                it = emit(it, val, spec, mesure_width);
            } else if (spec.align == '^') {
                size_t left_pad = padding / 2;
                size_t right_pad = padding - left_pad;

                for (size_t i = 0; i < left_pad; i++) {
                    *it++ = spec.fill;
                }
                it = emit(it, val, spec, mesure_width);
                for (size_t i = 0; i < right_pad; i++) {
                    *it++ = spec.fill;
                }
            } else {
                it = emit(ctx.iter, val, spec, mesure_width);
            }
            return it;
        } else {
            return emit(ctx.iter, val, spec, mesure_width);
        }
    }
};

}  // namespace nyan::lib::__format
