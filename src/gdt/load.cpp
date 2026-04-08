#include "load.hpp"

#include "entry.hpp"

namespace nyan::gdt {

alignas(16) constexpr static Segment entries[] = {
    makeSegment(0, 0, 0, 0),
    // 0x08 Code Ring0
    makeSegment(0,
                0xFFFFF,
                A_Present | A_Ring0 | A_NotSystem | A_Executable | A_CodeReadable | A_Accessed,
                F_Granularity | F_Size),
    // 0x10 Data Ring0
    makeSegment(0, 0xFFFFF, A_Present | A_Ring0 | A_NotSystem | A_DataWritable | A_Accessed, F_Granularity | F_Size),
    // 0x18 Code Ring3
    makeSegment(0,
                0xFFFFF,
                A_Present | A_Ring3 | A_NotSystem | A_Executable | A_CodeReadable | A_Accessed,
                F_Granularity | F_Size),
    // 0x20 Data Ring3
    makeSegment(0, 0xFFFFF, A_Present | A_Ring3 | A_NotSystem | A_DataWritable | A_Accessed, F_Granularity | F_Size),
    // TODO: tss
};

static Descriptor desc;

void load() {
    desc.size = sizeof(entries) - 1;
    desc.offset = reinterpret_cast<uint32_t>(entries);

    asm volatile("lgdt %0" ::"m"(desc) : "memory");

    asm volatile(
        "    ljmp $0x08, $.reload%=;"
        ".reload%=:;"
        "    mov $0x10, %%ax;"
        "    mov %%ax, %%ds;"
        "    mov %%ax, %%es;"
        "    mov %%ax, %%fs;"
        "    mov %%ax, %%gs;"
        "    mov %%ax, %%ss;" ::
            : "memory");
}

}  // namespace nyan::gdt
