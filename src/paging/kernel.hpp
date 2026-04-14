#pragma once

#include <stdint.h>

#include "directory.hpp"
#include "table.hpp"

namespace nyan::paging {

extern Directory kernelPageDirectory;
extern Table kernelPageTable[256];

void clearIdentityPaging();

struct MapperGuard {
    MapperGuard(PhysicalAddress addr) : paddr(addr) {
        vaddr = allocator::virtualFrameAlloc();
        kernelPageDirectory.map(
            {
                .pAddr = paddr,
                .vAddr = vaddr,
            },
            PTE_Present | PTE_ReadWrite);
        vaddr.invlpg();
    }
    MapperGuard(const MapperGuard&) = delete;
    MapperGuard(MapperGuard&& mapper) noexcept : paddr(mapper.paddr), vaddr(mapper.vaddr.addr) {
        mapper.paddr = {0};
        mapper.vaddr = {0};
    }
    ~MapperGuard() {
        if (vaddr) {
            PhysicalAddress _;
            kernelPageDirectory.unmap({vaddr}, _);
            vaddr.invlpg();
            allocator::virtualFrameFree(vaddr);
        }
    }
    MapperGuard& operator=(const MapperGuard&) = delete;
    MapperGuard& operator=(MapperGuard&& mapper) noexcept {
        if (this == &mapper) {
            return *this;
        }
        this->~MapperGuard();
        paddr = mapper.paddr;
        vaddr = mapper.vaddr;
        mapper.paddr = {0};
        mapper.vaddr = {0};
        return *this;
    }

    template <typename T>
    T* as() const noexcept {
        return vaddr.as<T>();
    }

    PhysicalAddress paddr;
    VirtualAddress vaddr;
};

}  // namespace nyan::paging
