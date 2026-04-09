#include "paging.hpp"

#include <stddef.h>
#include <algorithm>

#include "../paging/convert.hpp"
#include "../paging/directory.hpp"
#include "../paging/entry.hpp"
#include "../paging/table.hpp"

namespace nyan::setup {

paging::Directory pageDirectory;
paging::Table pageTable;

extern "C" void preparePaging() {
    auto pageDirectoryPtr = paging::virtualToPhysical(&pageDirectory);
    auto pageTablePtr = paging::virtualToPhysical(&pageTable);

    std::fill_n(pageDirectoryPtr->data, 1 << 10, 0);

    pageTablePtr->fillFlat(0, paging::PTE_Present | paging::PTE_ReadWrite);
    pageTablePtr->map(0xB8000, 0xC03FF000, paging::PTE_Present | paging::PTE_ReadWrite);

    pageDirectoryPtr->set(pageTablePtr, 0, paging::PDE_Present | paging::PDE_ReadWrite);
    pageDirectoryPtr->set(pageTablePtr, (3 << 8), paging::PDE_Present | paging::PDE_ReadWrite);

    pageDirectoryPtr->load();
    asm volatile(
        "movl %%cr0, %%eax;"
        "orl $0x80010000, %%eax;"
        "movl %%eax, %%cr0" ::
            : "eax", "memory");
}

void clearIdentityPaging() {
    pageDirectory.data[0] = 0;

    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;" ::
            : "eax", "memory");
}

}  // namespace nyan::setup
