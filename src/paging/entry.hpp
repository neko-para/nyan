#pragma once

#include <sys/types.h>

namespace nyan::paging {

enum PageDirectoryEntryFlags {
    PDE_Present = 1 << 0,
    PDE_ReadWrite = 1 << 1,
    PDE_User = 1 << 2,
    PDE_WriteThrough = 1 << 3,
    PDE_DisableCache = 1 << 4,
    PDE_Accessed = 1 << 5,
    PDE_LargePageSize = 1 << 7,
};

enum PageTableEntryFlags {
    PTE_Present = 1 << 0,
    PTE_ReadWrite = 1 << 1,
    PTE_User = 1 << 2,
    PTE_WriteThrough = 1 << 3,
    PTE_DisableCache = 1 << 4,
    PTE_Accessed = 1 << 5,
    PTE_Dirty = 1 << 6,
    PTE_Global = 1 << 8,
};

}  // namespace nyan::paging
