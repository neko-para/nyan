#include "load.hpp"

#include "entry.hpp"
#include "tss.hpp"

namespace nyan::gdt {

alignas(16) static Segment entries[] = {
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
    // 0x28 TSS
    {},
    // 0x30 TLS
    {},
};

static Descriptor desc;

Tss tss asm("tss");

void load() {
    tss.ss0 = 0x10;

    entries[5] = makeSegment(reinterpret_cast<uint32_t>(&tss), sizeof(tss), A_Present | A_Executable | A_Accessed, 0);

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
            : "ax", "memory");

    asm volatile(
        "movw $0x28, %%ax;"
        "ltr %%ax;" ::
            : "ax");
}

void setTls(const Segment& seg) {
    entries[6] = seg;
}

}  // namespace nyan::gdt
