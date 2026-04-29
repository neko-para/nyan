#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../lib/format.hpp"
#include "../logger/print.hpp"
#include "debug.hpp"

#define __CAPTURE_EIP()     \
    ({                      \
        void* __eip;        \
        asm volatile(       \
            "call 1f;"      \
            "1: popl %0;"   \
            : "=r"(__eip)); \
        __eip;              \
    })
namespace nyan::arch {

template <typename... Args>
void kprint(void* eip, lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    std::array<char, 256> buffer;
    auto final_iter = lib::format_to(lib::capped_iterator<256, std::array<char, 256>::iterator>{buffer.begin()}, fmt,
                                     std::forward<Args>(args)...);
    logger::emitLog(eip, logger::LL_Info,
                    std::string_view{buffer.begin(), buffer.begin() + std::min<size_t>(256, final_iter.__count)});
}

template <typename... Args>
[[noreturn]] void kfatal(void* eip, lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    kprint(eip, fmt, std::forward<Args>(args)...);
    kfatal(eip);
}

}  // namespace nyan::arch

#define kprint(...) kprint(__CAPTURE_EIP(), __VA_ARGS__)
#define kfatal(...) kfatal(__CAPTURE_EIP(), ##__VA_ARGS__)
