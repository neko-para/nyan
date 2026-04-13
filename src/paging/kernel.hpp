#pragma once

#include <stdint.h>

#include "directory.hpp"
#include "table.hpp"

namespace nyan::paging {

extern Directory kernelPageDirectory;
extern Table kernelPageTable[256];

void clearIdentityPaging();

struct MapperGuard {
    MapperGuard(uint32_t addr) : paddr(addr) {
        vaddr = allocator::virtualFrameAlloc();
        kernelPageDirectory.map(paddr, vaddr, PTE_Present | PTE_ReadWrite);
    }
    ~MapperGuard() {
        uint32_t _;
        kernelPageDirectory.unmap(vaddr, _);
        allocator::virtualFrameFree(vaddr);
    }

    template <typename T>
    T* frame() const noexcept {
        return reinterpret_cast<T*>(vaddr);
    }

    uint32_t paddr;
    uint32_t vaddr;
};

}  // namespace nyan::paging
