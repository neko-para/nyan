#include "kernel.hpp"

#include "convert.hpp"

namespace nyan::paging {

Directory kernelPageDirectory;
Table kernelPageTable[256];

void load() {
    kernelPageDirectory.clear();
    for (auto table : kernelPageTable) {
        table.clear();
    }
    for (size_t i = 0; i < 256; i++) {
        kernelPageDirectory.set(virtualToPhysical(kernelPageTable + i), i | (3 << 8), PDE_Present | PDE_ReadWrite);
    }
    kernelPageTable[0].fillFlat(0, paging::PTE_Present | paging::PTE_ReadWrite);
    kernelPageDirectory.map(0xB8000, 0xC03FF000, paging::PTE_Present | paging::PTE_ReadWrite);

    virtualToPhysical(&kernelPageDirectory)->load();
}

}  // namespace nyan::paging
