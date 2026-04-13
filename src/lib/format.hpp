#pragma once

#include <format>

#include "containers.hpp"
#include "format/formatter.hpp"
#include "format/utils.hpp"
#include "function.hpp"

namespace nyan::lib {

template <void (*put)(char ch)>
struct wrap_iterator {
    using difference_type = ptrdiff_t;

    wrap_iterator& operator=(char ch) noexcept {
        put(ch);
        return *this;
    }
    wrap_iterator& operator*() noexcept { return *this; }
    wrap_iterator& operator++() noexcept { return *this; }
    wrap_iterator& operator++(int) noexcept { return *this; }
};

template <typename... Args>
struct format_string {
    string_view fmt;

    template <typename... SvArgs>
        requires std::is_constructible_v<string_view, SvArgs...>
    consteval format_string(SvArgs&&... args) : fmt(std::forward<SvArgs>(args)...) {
        check<0>(fmt);
    }

    template <size_t I>
    consteval void check_arg(string_view sv) {
        if constexpr (I < sizeof...(Args)) {
            using T = std::remove_cvref_t<std::tuple_element_t<I, std::tuple<Args...>>>;

            __format::formatter<T, string_view::iterator> fmt;
            if (fmt.parse(sv) != sv.end()) {
                std::__throw_format_error("spec not fully consumed");
            }
        } else {
            std::__throw_format_error("args count not enough");
        }
    }

    template <size_t I>
    consteval void check(string_view sv) {
        if constexpr (I <= sizeof...(Args)) {
            for (auto it = sv.begin(); it != sv.end(); it++) {
                if (*it == '{') {
                    it++;
                    if (it == sv.end()) {
                        std::__throw_format_error("missing } before end");
                    } else if (*it == '{') {
                        continue;
                    } else {
                        auto right = it;
                        while (*right != '}') {
                            right++;
                            if (right == sv.end()) {
                                std::__throw_format_error("missing } before end");
                            }
                        }
                        check_arg<I>({it, right});
                        right++;
                        check<I + 1>({right, sv.end()});
                        return;
                    }
                } else if (*it == '}') {
                    auto next = it;
                    next++;
                    if (next == sv.end() || *next != '}') {
                        std::__throw_format_error("missing } before end for }}");
                    } else {
                        it++;
                    }
                }
            }
            if (I > sizeof...(Args)) {
                std::__throw_format_error("args count not enough");
            }
        } else {
            std::__throw_format_error("args count not enough");
        }
    }
};

template <__format::output_iterator Iter, typename... Args>
Iter format_to(Iter out, format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    auto&& data = std::forward_as_tuple(std::forward<Args>(args)...);
    function<void(string_view spec)> formatters[sizeof...(Args)];

    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((formatters[Is] =
              [&](string_view spec) {
                  using T = std::remove_cvref_t<std::tuple_element_t<Is, std::tuple<Args...>>>;

                  __format::formatter<T, string_view::iterator> fmt;
                  fmt.parse(spec);
                  __format::format_context ctx{out};
                  out = fmt.format(std::get<Is>(data), ctx);
              }),
         ...);
    }(std::make_index_sequence<sizeof...(Args)>());

    size_t count = 0;
    for (auto it = fmt.fmt.begin(); it != fmt.fmt.end(); it++) {
        if (*it == '{') {
            it++;
            if (it == fmt.fmt.end()) {
                std::__throw_format_error("missing } before end");
            } else if (*it == '{') {
                *out++ = '{';
                continue;
            } else {
                auto right = it;
                while (*right != '}') {
                    right++;
                    if (right == fmt.fmt.end()) {
                        std::__throw_format_error("missing } before end");
                    }
                }
                formatters[count++](string_view{it, right});
                it = right;
            }
        } else if (*it == '}') {
            auto next = it;
            next++;
            if (next == fmt.fmt.end() || *next != '}') {
                std::__throw_format_error("missing } before end for }}");
            } else {
                *out++ = '}';
            }
        } else {
            *out++ = *it;
        }
    }
    return out;
}

template <typename... Args>
[[nodiscard]] string format(format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    string result;
    result.reserve(fmt.fmt.size());
    format_to(std::back_insert_iterator<string>(result), fmt, std::forward<Args>(args)...);
    return result;
}

}  // namespace nyan::lib
