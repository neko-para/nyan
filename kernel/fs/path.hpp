#pragma once

#include <string>
#include <vector>

namespace nyan::fs {

struct Path {
    std::vector<std::string> __portions;
    bool __trailing_slash;
    bool __relative;
    bool __invalid;

    Path() noexcept : __trailing_slash(false), __relative(false), __invalid(true) {}
    Path(std::string_view path) noexcept;
    template <typename... Args>
    Path(bool trailing, bool relative, Args... args) noexcept
        : __portions(std::forward<Args>(args)...), __trailing_slash(trailing), __relative(relative), __invalid(false) {}

    Path parent() const noexcept;
    std::string last() const noexcept { return __portions.empty() ? "" : __portions.back(); }
};

}  // namespace nyan::fs
