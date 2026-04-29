#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../lib/format.hpp"
#include "../logger/print.hpp"
#include "debug.hpp"

namespace nyan::arch {

template <typename... Args>
void kprint(lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    std::array<char, 256> buffer;
    auto final_iter = lib::format_to(lib::capped_iterator<256, std::array<char, 256>::iterator>{buffer.begin()}, fmt,
                                     std::forward<Args>(args)...);
    logger::emitLog(__builtin_return_address(0), logger::LL_Info,
                    std::string_view{buffer.begin(), buffer.begin() + final_iter.__count});
}

template <typename... Args>
[[noreturn]] void kfatal(lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    kprint(fmt, std::forward<Args>(args)...);
    kfatal();
}

}  // namespace nyan::arch
