#include "buffer.hpp"

#include <algorithm>

#include "../vga/cursor.hpp"

namespace nyan::tty {

void ScreenBuffer::flush() {
    flushBuffer();
    flushCursor();
}

void ScreenBuffer::flushBuffer() {
    if (!(flags & F_Active)) {
        return;
    }
    std::copy(std::begin(buffer), std::end(buffer), std::begin(vga::buffer));
}

void ScreenBuffer::flushCursor() {
    if (!(flags & F_Active)) {
        return;
    }
    if (flags & (F_ShowCursor)) {
        vga::showCursor();
        vga::updateCursor(colPtr, rowPtr);
    } else {
        vga::hideCursor();
    }
}

void ScreenBuffer::clear() {
    std::fill_n(buffer, width * height, vga::Entry{0, currentAttr});
}

void ScreenBuffer::scroll(size_t row) {
    size_t offset = row * width;
    size_t count = height * width - offset;
    std::copy_n(buffer + offset, count, buffer);
    std::fill_n(buffer + count, offset, vga::Entry{0, currentAttr});
}

void ScreenBuffer::putcImpl(char ch) {
    if (ch == '\n') {
        goto putLF;
    } else if (ch == '\r') {
        colPtr = 0;
        return;
    } else if (ch == '\b') {
        if (colPtr > 0) {
            colPtr -= 1;
        }
        return;
    } else if (ch == '\t') {
        colPtr += 4 - (colPtr & 3);
        return;
    }

    buffer[rowPtr * width + colPtr].ch = ch;
    if (flags & F_Active) {
        vga::buffer[rowPtr * width + colPtr].ch = ch;
    }

    if (++colPtr == width) {
    putLF:
        colPtr = 0;
        if (++rowPtr == height) {
            scroll(1);
            rowPtr = height - 1;
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

}  // namespace nyan::tty
