#pragma once

#include "instr.hpp"

#include "../arch/guard.hpp"

namespace nyan::arch {

inline void enableSse() {
    uint32_t cr0, cr4;
    asm volatile("movl %%cr0, %0" : "=a"(cr0));
    cr0 &= 0xFFFFFFFB;
    cr0 |= 0x2;
    asm volatile("movl %0, %%cr0" ::"a"(cr0));

    asm volatile("movl %%cr4, %0" : "=a"(cr4));
    cr4 |= 3 << 9;
    asm volatile("movl %0, %%cr4" ::"a"(cr4));
}

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
