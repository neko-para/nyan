#include <stddef.h>

#include "../lib/algorithm.hpp"
#include "../paging/convert.hpp"
#include "../paging/entry.hpp"

namespace nyan::setup {

alignas(4096) __attribute__((section(".bss"))) uint32_t pageDirectory[1 << 10];
alignas(4096) __attribute__((section(".bss"))) uint32_t pageTable[1 << 10];

extern "C" void preparePaging() {
    uint32_t* pageDirectoryPtr = paging::virtualToPhysical(pageDirectory);
    uint32_t* pageTablePtr = paging::virtualToPhysical(pageTable);

    lib::fill_n(pageDirectoryPtr, 1 << 10, 0);

    for (size_t i = 0; i < ((1 << 10) - 1); i++) {
        pageTablePtr[i] = (i << 12) | paging::PTE_Present | paging::PTE_ReadWrite;
    }
    pageTablePtr[(1 << 10) - 1] = 0xB8000 | paging::PTE_Present | paging::PTE_ReadWrite;

    pageDirectoryPtr[0] = reinterpret_cast<uint32_t>(pageTablePtr) | paging::PTE_Present | paging::PTE_ReadWrite;
    pageDirectoryPtr[3 << 8] = reinterpret_cast<uint32_t>(pageTablePtr) | paging::PTE_Present | paging::PTE_ReadWrite;

    asm volatile("movl %0, %%cr3" ::"r"(pageDirectoryPtr) : "memory");
    asm volatile(
        "movl %%cr0, %%eax;"
        "orl $0x80010000, %%eax;"
        "movl %%eax, %%cr0" ::
            : "eax", "memory");
}

void clearIdentityPaging() {
    pageDirectory[0] = 0;

    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;" ::
            : "eax", "memory");
}

}  // namespace nyan::setup
