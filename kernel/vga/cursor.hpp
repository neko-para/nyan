#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../arch/port.hpp"
#include "buffer.hpp"

namespace nyan::vga {

inline void showCursor(uint8_t cursor_start, uint8_t cursor_end) {
    arch::outb(0x3D4, 0x0A);
    arch::outb(0x3D5, (arch::inb(0x3D5) & 0xC0) | cursor_start);

    arch::outb(0x3D4, 0x0B);
    arch::outb(0x3D5, (arch::inb(0x3D5) & 0xE0) | cursor_end);
}

inline void showCursor() {
    showCursor(14, 15);
}

inline void hideCursor() {
    arch::outb(0x3D4, 0x0A);
    arch::outb(0x3D5, 0x20);
}

inline void updateCursor(size_t col, size_t row) {
    uint16_t pos = row * width + col;

    arch::outb(0x3D4, 0x0F);
    arch::outb(0x3D5, (uint8_t)(pos & 0xFF));
    arch::outb(0x3D4, 0x0E);
    arch::outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

}  // namespace nyan::vga
