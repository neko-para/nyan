#pragma once

#include <stddef.h>
#include <stdint.h>

namespace nyan::vga {

constexpr uint8_t makeAttr(uint8_t fg, uint8_t bg) {
    return (bg << 4) | fg;
}

extern uint8_t currentAttr;

void clear();
void putc(char ch);
void puts(const char* str);
void puts(const char* str, size_t len);

}  // namespace nyan::vga
