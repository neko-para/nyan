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
        asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
    }
    MapperGuard(void* addr) : MapperGuard(reinterpret_cast<uint32_t>(addr)) {}
    MapperGuard(const MapperGuard&) = delete;
    MapperGuard(MapperGuard&& mapper) noexcept : paddr(mapper.paddr), vaddr(mapper.vaddr) {
        mapper.paddr = 0;
        mapper.vaddr = 0;
    }
    ~MapperGuard() {
        if (vaddr) {
            uint32_t _;
            kernelPageDirectory.unmap(vaddr, _);
            asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
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
        mapper.paddr = 0;
        mapper.vaddr = 0;
        return *this;
    }

    template <typename T>
    T* frame() const noexcept {
        return reinterpret_cast<T*>(vaddr);
    }

    uint32_t paddr;
    uint32_t vaddr;
};

}  // namespace nyan::paging
