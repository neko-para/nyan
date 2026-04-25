#pragma once

#include <stddef.h>
#include <stdint.h>

namespace nyan::interrupt {

struct __attribute__((packed)) Descriptor {
    uint16_t size;
    uint32_t offset;
};

enum Attribute {
    A_GateTask = 5,
    A_GateInterrupt16 = 6,
    A_GateTrap16 = 7,
    A_GateInterrupt = 14,
    A_GateTrap = 15,
    A_Ring0 = 0 << 5,
    A_Ring1 = 1 << 5,
    A_Ring2 = 2 << 5,
    A_Ring3 = 3 << 5,
    A_Present = 1 << 7,
};

struct Entry {
    uint16_t isr_lo;
    uint16_t kernel_cs;
    uint8_t _;
    uint8_t attr;
    uint16_t isr_hi;
};

inline Entry makeEntry(uint32_t isr, uint16_t cs, uint8_t attr) {
    Entry entry;
    entry.isr_lo = isr & 0xFFFF;
    entry.kernel_cs = cs;
    entry._ = 0;
    entry.attr = attr;
    entry.isr_hi = (isr >> 16) & 0xFFFF;
    return entry;
}

}  // namespace nyan::interrupt
