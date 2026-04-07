#include "print.hpp"

#include <algorithm>

#include "buffer.hpp"

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

void putc(char ch) {
    at(rowPtr, colPtr).ch = ch;

    if (++colPtr == width) {
        colPtr = 0;
        if (++rowPtr == height) {
            // TODO: scroll
            rowPtr = 0;
        }
    }
}

void puts(const char* str) {
    while (*str) {
        putc(*str++);
    }
}

void puts(const char* str, size_t len) {
    while (len-- > 0) {
        putc(*str++);
    }
}

}  // namespace nyan::vga
