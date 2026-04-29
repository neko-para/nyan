#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <source_location>

#include "../lib/format.hpp"
#include "debug.hpp"
#include "guard.hpp"

namespace nyan::arch {

pid_t __safe_getpid();

template <typename... Args>
struct log_format_string {
    lib::format_string<Args...> __format;
    std::source_location __location;

    template <typename SvArgs>
        requires std::is_constructible_v<lib::format_string<Args...>, SvArgs>
    consteval log_format_string(SvArgs&& args, const std::source_location& loc = std::source_location::current())
        : __format(std::forward<SvArgs>(args)), __location(loc) {}
};

template <typename... Args>
void kprint(log_format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    std::array<char, 256> buffer;
    auto final_iter = lib::format_to(lib::capped_iterator<256, std::array<char, 256>::iterator>{buffer.begin()},
                                     fmt.__format, std::forward<Args>(args)...);
    InterruptGuard guard;
    lib::format_to(lib::wrap_iterator<&kput>{}, "{}:{}:{} ", fmt.__location.file_name(), fmt.__location.line(),
                   __safe_getpid());
    kputs(buffer.data(), final_iter.__count);
}

template <typename... Args>
[[noreturn]] void kfatal(log_format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    // 都出错了就直接关中断了
    InterruptGuard guard;
    kprint(fmt, std::forward<Args>(args)...);
    kfatal();
}

}  // namespace nyan::arch
