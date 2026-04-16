#pragma once

#include <sys/types.h>

#include "../lib/format.hpp"

namespace nyan::vga {

constexpr uint8_t makeAttr(uint8_t fg, uint8_t bg) {
    return (bg << 4) | fg;
}

extern uint8_t currentAttr;

void clear();
void scroll(size_t row);
__attribute__((noinline)) void putcImpl(char ch);
void flushCursor();
void putc(char ch);
void puts(const char* str);
void puts(const char* str, size_t len);

template <typename... Args>
void print(lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
    lib::format_to(lib::wrap_iterator<&putcImpl>{}, fmt, std::forward<Args>(args)...);
    flushCursor();
}

}  // namespace nyan::vga
