#pragma once

#include <stdint.h>

#include "../lib/format.hpp"
#include "utils.hpp"

namespace nyan::arch {

template <typename... Args>
[[noreturn]] void kfatal(lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    lib::format_to(lib::wrap_iterator<&kput>{}, fmt, std::forward<Args>(args)...);
    kfatal();
}

}  // namespace nyan::arch
