#pragma once

#include "../lib/format.hpp"
#include "../vga/buffer.hpp"
#include "../vga/entry.hpp"

namespace nyan::tty {

constexpr size_t width = vga::width;
constexpr size_t height = vga::height;

enum Flags {
    F_Active = 1 << 0,
    F_ShowCursor = 1 << 1,
    F_Canonical = 1 << 2,
    F_Echo = 1 << 4,
};

struct ScreenBuffer {
    vga::Entry buffer[width * height];

    int rowPtr = 0;
    int colPtr = 0;
    uint8_t currentAttr;
    uint8_t flags;

    void flush();
    void flushBuffer();
    void flushCursor();
    void clear();
    void scroll(size_t row);
    void putcImpl(char ch);

    void putc(char ch);
    void puts(const char* str);
    void puts(const char* str, size_t len);

    template <typename... Args>
    void print(lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) {
        lib::format_to(lib::wrap_member_iterator<ScreenBuffer, &ScreenBuffer::putcImpl>{this}, fmt,
                       std::forward<Args>(args)...);
        flushCursor();
    }
};

}  // namespace nyan::tty
