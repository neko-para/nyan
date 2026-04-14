#include "kernel.hpp"

#include "directory.hpp"
#include "table.hpp"

namespace nyan::paging {

KernelDirectory kernelPageDirectory;
Table kernelPageTable[256];

// RUN IN LOWER
extern "C" void preparePaging() {
    auto cr3Addr = paging::VirtualAddress(&kernelPageDirectory).kernelToPhysical();
    auto& phyDirectory = *cr3Addr.unsafeAs<KernelDirectory>();

    auto tableAddr = paging::VirtualAddress{kernelPageTable}.kernelToPhysical();
    auto& phyTable = *tableAddr.unsafeAs<Table[256]>();

    phyDirectory.clear();
    for (auto& table : phyTable) {
        table.clear();
    }

    for (size_t i = 0; i < 256; i++) {
        PhysicalAddress addr = {
            tableAddr.addr + i * sizeof(Table),
        };
        phyDirectory.set(addr, i, PDE_Present | PDE_ReadWrite);
        phyDirectory.set(addr, i | (3 << 8), PDE_Present | PDE_ReadWrite);
    }
    phyTable[0].fillFlat(0, paging::PTE_Present | paging::PTE_ReadWrite);
    phyTable[0].map({.pAddr = {0xB8000}, .vAddr = {0xC03FF000}}, paging::PTE_Present | paging::PTE_ReadWrite);

    cr3Addr.setCr3();

    asm volatile(
        "movl %%cr0, %%eax;"
        "orl $0x80010000, %%eax;"
        "movl %%eax, %%cr0" ::
            : "eax", "memory");
}

void clearIdentityPaging() {
    for (size_t i = 0; i < 256; i++) {
        kernelPageDirectory.set({0}, i, 0);
    }

    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;" ::
            : "eax", "memory");
}

}  // namespace nyan::paging
