#pragma once

#include <stdint.h>

namespace nyan::arch {

inline uint32_t flags() {
    uint32_t f;
    asm volatile(
        "pushfl;"
        "popl %0;"
        : "=r"(f)::"memory");
    return f;
}

inline void setFlags(uint32_t f) {
    asm volatile(
        "pushl %0;"
        "popfl;" ::"r"(f)
        : "memory");
}

inline uint32_t cr2() {
    uint32_t reg;
    asm volatile("movl %%cr2, %0;" : "=r"(reg)::"memory");
    return reg;
}

inline void setCr3(uint32_t addr) {
    asm volatile("movl %0, %%cr3" ::"r"(addr) : "memory");
}

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

inline void invlpg(uint32_t addr) {
    asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

}  // namespace nyan::arch
