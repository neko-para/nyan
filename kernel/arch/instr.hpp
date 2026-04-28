#pragma once

#include <stdint.h>
#include <sys/types.h>

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

inline void invlpg(uint32_t addr) {
    asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

}  // namespace nyan::arch
