#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

namespace nyan::arch {

inline void cli() {
    asm volatile("cli");
}

inline void sti() {
    asm volatile("sti");
}

inline void hlt() {
    asm volatile("hlt");
}

inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

inline void outw(uint16_t port, uint16_t val) {
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

inline void outl(uint16_t port, uint32_t val) {
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

inline void kput(char ch) {
    outb(0xe9, ch);
}

inline void kputs(const char* str) {
    while (*str) {
        kput(*str++);
    }
}

[[noreturn]] inline void kfatal(const char* str) {
    kputs(str);
    cli();
    for (;;) {
        hlt();
    }
}

[[noreturn]] __attribute__((__format__(__printf__, 1, 2))) inline void kfatalfmt(const char* fmt, ...) {
    static char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, 256, fmt, ap);
    va_end(ap);

    kfatal(buf);
}

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

}  // namespace nyan::arch
