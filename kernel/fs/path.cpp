#include "path.hpp"

#include <ranges>

namespace nyan::fs {

Path::Path(std::string_view path) noexcept {
    __trailing_slash = path.ends_with('/');
    __relative = !path.starts_with('/');
    __invalid = path.empty();
    for (const auto& item : path | std::views::split('/')) {
        auto portion = std::string_view{item.begin(), item.end()};
        if (portion.empty() || portion == ".") {
            continue;
        }
        __portions.push_back(std::string{portion});
    }
}

Path Path::parent() const noexcept {
    if (__portions.empty()) {
        return Path{__trailing_slash, __relative};
    } else {
        return Path{__trailing_slash, __relative, __portions.begin(), std::prev(__portions.end())};
    }
}

}  // namespace nyan::fs
