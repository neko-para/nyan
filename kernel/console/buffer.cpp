#include "buffer.hpp"

#include <algorithm>

#include "../vga/cursor.hpp"

namespace nyan::console {

void ScreenBuffer::flush() {
    flushBuffer();
    flushCursor();
}

void ScreenBuffer::flushBuffer() {
    if (!(__flags & F_Active)) {
        return;
    }
    std::copy(std::begin(__buffer), std::end(__buffer), std::begin(vga::buffer));
}

void ScreenBuffer::flushCursor() {
    if (!(__flags & F_Active)) {
        return;
    }
    if (__flags & (F_ShowCursor)) {
        vga::showCursor();
        vga::updateCursor(__col_ptr, __row_ptr);
    } else {
        vga::hideCursor();
    }
}

void ScreenBuffer::clear() {
    std::fill_n(__buffer, __width * __height, vga::Entry{0, __current_attr});
}

void ScreenBuffer::scroll(size_t row) {
    size_t offset = row * __width;
    size_t count = __height * __width - offset;
    std::copy_n(__buffer + offset, count, __buffer);
    std::fill_n(__buffer + count, offset, vga::Entry{0, __current_attr});
}

void ScreenBuffer::putcImpl(char ch) {
    if (ch == '\n') {
        goto putLF;
    } else if (ch == '\r') {
        __col_ptr = 0;
        return;
    } else if (ch == '\b') {
        if (__col_ptr > 0) {
            __col_ptr -= 1;
        }
        return;
    } else if (ch == '\t') {
        __col_ptr += 4 - (__col_ptr & 3);
        if (__col_ptr == __width) {
            goto putLF;
        }
        return;
    }

    __buffer[__row_ptr * __width + __col_ptr].ch = ch;
    if (__flags & F_Active) {
        vga::buffer[__row_ptr * __width + __col_ptr].ch = ch;
    }

    if (++__col_ptr == __width) {
    putLF:
        __col_ptr = 0;
        if (++__row_ptr == __height) {
            scroll(1);
            __row_ptr = __height - 1;
            flushBuffer();
        }
    }
}

void ScreenBuffer::putc(char ch) {
    putcImpl(ch);
    flushCursor();
}

void ScreenBuffer::puts(const char* str) {
    while (*str) {
        putcImpl(*str++);
    }
    flushCursor();
}

void ScreenBuffer::puts(const char* str, size_t len) {
    while (len-- > 0) {
        putcImpl(*str++);
    }
    flushCursor();
}

}  // namespace nyan::console
