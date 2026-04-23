#pragma once

#include <sys/types.h>

#include "../lib/format.hpp"
#include "debug.hpp"
#include "guard.hpp"

namespace nyan::arch {

template <typename... Args>
void kprint(lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    InterruptGuard guard;
    lib::format_to(lib::wrap_iterator<&kput>{}, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
[[noreturn]] void kfatal(lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    lib::format_to(lib::wrap_iterator<&kput>{}, fmt, std::forward<Args>(args)...);
    kfatal();
}

}  // namespace nyan::arch
