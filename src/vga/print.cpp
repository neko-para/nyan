#include "print.hpp"

#include <algorithm>

#include "buffer.hpp"
#include "cursor.hpp"

namespace nyan::vga {

uint8_t currentAttr = makeAttr(C_LightGray, C_Black);

int rowPtr = 0;
int colPtr = 0;

static inline volatile Entry& at(int row, int col) {
    return buffer[row * width + col];
}

void clear() {
    std::fill_n(buffer, width * height, Entry{0, currentAttr});
}

void scroll(size_t row) {
    size_t offset = row * width;
    size_t count = height * width - offset;
    std::copy_n(buffer + offset, count, buffer);
    std::fill_n(buffer + count, offset, Entry{0, currentAttr});
}

void putcImpl(char ch) {
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

    at(rowPtr, colPtr).ch = ch;

    if (++colPtr == width) {
    putLF:
        colPtr = 0;
        if (++rowPtr == height) {
            scroll(1);
            rowPtr = height - 1;
        }
    }
}

void putc(char ch) {
    putcImpl(ch);
    updateCursor(colPtr, rowPtr);
}

void puts(const char* str) {
    while (*str) {
        putcImpl(*str++);
    }
    updateCursor(colPtr, rowPtr);
}

void puts(const char* str, size_t len) {
    while (len-- > 0) {
        putcImpl(*str++);
    }
    updateCursor(colPtr, rowPtr);
}

}  // namespace nyan::vga
