#pragma once

#include "../lib/format.hpp"
#include "../vga/buffer.hpp"
#include "../vga/entry.hpp"

namespace nyan::console {

// TODO: 大概应该改成Tty上的状态
constexpr size_t __width = vga::width;
constexpr size_t __height = vga::height;

enum Flags {
    F_Active = 1 << 0,
    F_ShowCursor = 1 << 1,
    F_Canonical = 1 << 2,
    F_Echo = 1 << 4,
};

enum VTState {
    VTS_Idle,
    VTS_Esc,
    VTS_EscBrc,
};

struct ScreenBuffer {
    vga::Entry __buffer[__width * __height];

    int __row_ptr = 0;
    int __col_ptr = 0;
    uint8_t __current_attr = vga::makeAttr(vga::C_LightGray, vga::C_Black);
    uint8_t __flags = F_ShowCursor | F_Canonical | F_Echo;

    VTState __vtstate = VTS_Idle;
    int __vtarg{};
    std::vector<int> __vtargs;

    void flush() noexcept;
    void flushBuffer() noexcept;
    void flushCursor() noexcept;
    void clear() noexcept;
    void scroll(size_t row) noexcept;
    void putcImpl(char ch) noexcept;

    void renderChar(char ch) noexcept;
    void putc(char ch) noexcept;
    void puts(const char* str) noexcept;
    void puts(const char* str, size_t len) noexcept;

    void flushCSI(char cmd) noexcept;

    template <typename... Args>
    void print(lib::format_string<std::type_identity_t<Args>...> fmt, Args&&... args) noexcept {
        lib::format_to(lib::wrap_member_iterator<ScreenBuffer, &ScreenBuffer::putcImpl>{this}, fmt,
                       std::forward<Args>(args)...);
        flushCursor();
    }
};

}  // namespace nyan::console
