#pragma once

#include <sys/types.h>

namespace nyan::vga {

enum Color {
    C_Black = 0,
    C_Blue = 1,
    C_Green = 2,
    C_Cyan = 3,
    C_Red = 4,
    C_Magenta = 5,
    C_Brown = 6,
    C_LightGray = 7,
    C_DarkGray = 8,
    C_LightBlue = 9,
    C_LightGreen = 10,
    C_LightCyan = 11,
    C_LightRed = 12,
    C_LightMagenta = 13,
    C_Yellow = 14,
    C_White = 15,
};

struct Entry {
    char ch;
    uint8_t attr;

    volatile Entry& operator=(const Entry& entry) volatile {
        value() = entry.value();
        return *this;
    }
    volatile Entry& operator=(const volatile Entry& entry) volatile {
        value() = entry.value();
        return *this;
    }

    const uint16_t& value() const& { return *reinterpret_cast<const uint16_t*>(this); }
    uint16_t& value() & { return *reinterpret_cast<uint16_t*>(this); }
    uint16_t value() const&& { return *reinterpret_cast<const uint16_t*>(this); }

    const volatile uint16_t& value() const volatile& { return *reinterpret_cast<const volatile uint16_t*>(this); }
    volatile uint16_t& value() volatile& { return *reinterpret_cast<volatile uint16_t*>(this); }
};

}  // namespace nyan::vga
