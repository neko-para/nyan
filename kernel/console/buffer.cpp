#include "buffer.hpp"

#include <algorithm>

#include "../arch/print.hpp"
#include "../vga/cursor.hpp"

namespace nyan::console {

void ScreenBuffer::flush() noexcept {
    flushBuffer();
    flushCursor();
}

void ScreenBuffer::flushBuffer() noexcept {
    if (!(__flags & F_Active)) {
        return;
    }
    std::copy(std::begin(__buffer), std::end(__buffer), std::begin(vga::buffer));
}

void ScreenBuffer::flushCursor() noexcept {
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

void ScreenBuffer::clear() noexcept {
    std::fill_n(__buffer, __width * __height, vga::Entry{0, __current_attr});
}

void ScreenBuffer::scroll(size_t row) noexcept {
    size_t offset = row * __width;
    size_t count = __height * __width - offset;
    std::copy_n(__buffer + offset, count, __buffer);
    std::fill_n(__buffer + count, offset, vga::Entry{0, __current_attr});
}

void ScreenBuffer::renderChar(char ch) noexcept {
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

    __buffer[__row_ptr * __width + __col_ptr] = {ch, __current_attr};
    if (__flags & F_Active) {
        vga::buffer[__row_ptr * __width + __col_ptr] = {ch, __current_attr};
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

void ScreenBuffer::putcImpl(char ch) noexcept {
    switch (__vtstate) {
        case VTS_Idle:
            if (ch == '\e') {
                __vtstate = VTS_Esc;
            } else {
                renderChar(ch);
            }
            break;
        case VTS_Esc:
            if (ch == '[') {
                __vtstate = VTS_EscBrc;
                __vtarg = 0;
                __vtargs = {};
            } else {
                __vtstate = VTS_Idle;
                renderChar(ch);
            }
            break;
        case VTS_EscBrc:
            if (ch >= '0' && ch <= '9') {
                __vtarg = __vtarg * 10 + (ch - '0');
            } else if (ch == ';') {
                __vtargs.push_back(__vtarg);
                __vtarg = 0;
            } else {
                __vtargs.push_back(__vtarg);
                flushCSI(ch);
                __vtstate = VTS_Idle;
            }
            break;
    }
}

void ScreenBuffer::putc(char ch) noexcept {
    putcImpl(ch);
    flushCursor();
}

void ScreenBuffer::puts(const char* str) noexcept {
    while (*str) {
        putcImpl(*str++);
    }
    flushCursor();
}

void ScreenBuffer::puts(const char* str, size_t len) noexcept {
    while (len-- > 0) {
        putcImpl(*str++);
    }
    flushCursor();
}

void ScreenBuffer::flushCSI(char cmd) noexcept {
    int arg0 = __vtargs[0];
    int arg1 = __vtargs.size() > 1 ? __vtargs[1] : 0;
    switch (cmd) {
        case 'm': {
            constexpr uint8_t ansiToVga[8] = {0, 4, 2, 6, 1, 5, 3, 7};

            for (auto param : __vtargs) {
                if (param == 0) {
                    // Reset
                    __current_attr = vga::makeAttr(vga::C_LightGray, vga::C_Black);
                } else if (param == 1) {
                    // Bold → 亮色 (bit 3)
                    __current_attr |= 0x08;
                } else if (param == 22) {
                    // Normal intensity
                    __current_attr &= ~0x08;
                } else if (param >= 30 && param <= 37) {
                    // 前景色
                    uint8_t bg = __current_attr & 0xF0;
                    uint8_t bright = __current_attr & 0x08;
                    __current_attr = bg | bright | ansiToVga[param - 30];
                } else if (param == 39) {
                    // 默认前景
                    uint8_t bg = __current_attr & 0xF0;
                    __current_attr = bg | vga::C_LightGray;
                } else if (param >= 40 && param <= 47) {
                    // 背景色
                    uint8_t fg = __current_attr & 0x0F;
                    __current_attr = (ansiToVga[param - 40] << 4) | fg;
                } else if (param == 49) {
                    // 默认背景
                    uint8_t fg = __current_attr & 0x0F;
                    __current_attr = fg;  // bg = 0 (Black)
                } else if (param >= 90 && param <= 97) {
                    // 亮前景色
                    uint8_t bg = __current_attr & 0xF0;
                    __current_attr = bg | 0x08 | ansiToVga[param - 90];
                } else if (param >= 100 && param <= 107) {
                    // 亮背景色
                    uint8_t fg = __current_attr & 0x0F;
                    __current_attr = ((ansiToVga[param - 100] | 0x08) << 4) | fg;
                }
            }
            break;
        }
        case 'H':
            __row_ptr = std::clamp<int>(arg0, 1, __height) - 1;
            __col_ptr = std::clamp<int>(arg1, 1, __width) - 1;
            break;
        case 'J': {
            size_t curr = __row_ptr * __width + __col_ptr;

            size_t fill_start = 0;
            size_t fill_count = 0;

            switch (arg0) {
                case 0:  // clear to end
                    fill_start = curr;
                    fill_count = __width * __height - curr;
                    break;
                case 1:  // clear to start
                    fill_start = 0;
                    fill_count = curr;
                    break;
                case 2:
                    fill_start = 0;
                    fill_count = __width * __height;
                    break;
            }

            vga::Entry fill = {' ', __current_attr};
            std::fill_n(&__buffer[fill_start], fill_count, fill);
            if (__flags & F_Active) {
                std::fill_n(&vga::buffer[fill_start], fill_count, fill);
            }

            break;
        }
        default:
            arch::kprint("unknown CSI {}", cmd);
    }
}

}  // namespace nyan::console
