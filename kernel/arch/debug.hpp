#pragma once

#include "guard.hpp"
#include "port.hpp"

namespace nyan::arch {

inline void qemuQuit() {
    outw(0x604, 0x2000);
}

inline void kput(char ch) {
    outb(0xe9, ch);
}

inline void kputs(const char* str) {
    while (*str) {
        kput(*str++);
    }
}

inline void kputs(const char* str, size_t len) {
    arch::InterruptGuard guard;
    while (len--) {
        kput(*str++);
    }
}

[[noreturn]] inline void kfatal() {
    cli();
    for (;;) {
        hlt();
    }
}

}  // namespace nyan::arch
