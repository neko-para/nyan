#include "kernel.hpp"

#include "convert.hpp"

namespace nyan::paging {

Directory kernelPageDirectory;
Table kernelPageTable[256];

// RUN IN LOWER
extern "C" void preparePaging() {
    auto& phyDirectory = *virtualToPhysical(&kernelPageDirectory);
    auto& phyTable = *virtualToPhysical(&kernelPageTable);

    phyDirectory.clear();
    for (auto& table : phyTable) {
        table.clear();
    }

    for (size_t i = 0; i < 256; i++) {
        phyDirectory.set(phyTable + i, i, PDE_Present | PDE_ReadWrite);
        phyDirectory.set(phyTable + i, i | (3 << 8), PDE_Present | PDE_ReadWrite);
    }
    phyTable[0].fillFlat(0, paging::PTE_Present | paging::PTE_ReadWrite);
    phyTable[0].map(0xB8000, 0xC03FF000, paging::PTE_Present | paging::PTE_ReadWrite);

    phyDirectory.load();

    asm volatile(
        "movl %%cr0, %%eax;"
        "orl $0x80010000, %%eax;"
        "movl %%eax, %%cr0" ::
            : "eax", "memory");
}

void clearIdentityPaging() {
    for (size_t i = 0; i < 256; i++) {
        kernelPageDirectory.set(0, i, 0);
    }

    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;" ::
            : "eax", "memory");
}

}  // namespace nyan::paging
