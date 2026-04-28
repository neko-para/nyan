#pragma once

#include <stdint.h>
#include <sys/types.h>

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

}  // namespace nyan::arch
