#pragma once

#include <stddef.h>
#include <stdint.h>

namespace nyan::gdt {

struct __attribute__((packed)) Descriptor {
    uint16_t size;
    uint32_t offset;
};

enum Access {
    A_Accessed = 1 << 0,
    A_DataWritable = 1 << 1,
    A_CodeReadable = 1 << 1,
    A_DataDirection = 1 << 2,
    A_CodeConforming = 1 << 2,
    A_Executable = 1 << 3,
    A_NotSystem = 1 << 4,
    A_Ring0 = 0 << 5,
    A_Ring1 = 1 << 5,
    A_Ring2 = 2 << 5,
    A_Ring3 = 3 << 5,
    A_Present = 1 << 7,
};

enum Flags {
    F_Long = 1 << 1,
    F_Size = 1 << 2,
    F_Granularity = 1 << 3,
};

struct Segment {
    uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t base_md;
    uint8_t access;
    uint8_t flags_limit_hi;
    uint8_t base_hi;
};

constexpr Segment makeSegment(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    Segment seg;
    seg.limit_lo = limit & 0xFFFF;
    seg.base_lo = base & 0xFFFF;
    seg.base_md = (base >> 16) & 0xFF;
    seg.access = access;
    seg.flags_limit_hi = ((limit >> 16) & 0xF) | ((flags & 0xF) << 4);
    seg.base_hi = (base >> 24) & 0xFF;
    return seg;
}

}  // namespace nyan::gdt
