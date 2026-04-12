#pragma once

#include <format>

#include "containers.hpp"
#include "format/utils.hpp"
#include "function.hpp"

namespace nyan::lib {

template <typename... Args>
struct format_string {
    string_view fmt;

    template <typename... SvArgs>
        requires std::is_constructible_v<string_view, SvArgs...>
    consteval format_string(SvArgs&&... args) : fmt(std::forward<SvArgs>(args)...) {
        check();
    }

    consteval bool check() {
        size_t count = 0;
        for (auto it = fmt.begin(); it != fmt.end(); it++) {
            if (*it == '{') {
                it++;
                if (it == fmt.end()) {
                    std::__throw_format_error("missing }");
                } else if (*it == '{') {
                    continue;
                } else {
                    auto right = it;
                    while (*right != '}') {
                        right++;
                        if (right == fmt.end()) {
                            std::__throw_format_error("missing }");
                        }
                    }
                    count++;
                }
            } else if (*it == '}') {
                auto next = it;
                next++;
                if (next == fmt.end() || *next != '}') {
                    std::__throw_format_error("missing }");
                } else {
                    it++;
                }
            }
        }
        if (count > sizeof...(Args)) {
            std::__throw_format_error("args count not enough");
        }
        return true;
    }
};

template <__format::output_iterator Iter, typename... Args>
Iter format_to(Iter out, format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    auto&& data = std::forward_as_tuple(std::forward<Args>(args)...);
    std::vector<function<void(string_view spec)>> formatters;

    [&]<size_t... Is>(std::index_sequence<Is...>) {
        (formatters.push_back([&](string_view) { out = __format::toChars(out, std::get<Is>(data)); }), ...);
    }(std::make_index_sequence<sizeof...(Args)>());

    size_t count = 0;
    for (auto it = fmt.fmt.begin(); it != fmt.fmt.end(); it++) {
        if (*it == '{') {
            it++;
            if (it == fmt.fmt.end()) {
                std::__throw_format_error("missing }");
            } else if (*it == '{') {
                *out++ = '{';
                continue;
            } else {
                auto right = it;
                while (*right != '}') {
                    right++;
                    if (right == fmt.fmt.end()) {
                        std::__throw_format_error("missing }");
                    }
                }
                formatters[count++](string_view{it, right});
            }
        } else if (*it == '}') {
            auto next = it;
            next++;
            if (next == fmt.fmt.end() || *next != '}') {
                std::__throw_format_error("missing }");
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
