#pragma once

#include <sys/types.h>
#include <algorithm>

#include "../allocator/utils.hpp"
#include "address.hpp"
#include "entry.hpp"

namespace nyan::paging {

struct alignas(4096) Table {
    uint32_t data[1024];

    void clear() noexcept { std::fill_n(data, 1024, 0); }
    void fillFlat(uint32_t base, uint16_t attr) noexcept {
        for (size_t i = 0; i < 1024; i++) {
            data[i] = (base + (i << 12)) | attr;
        }
    }

    void map(PairedAddress addrs, uint16_t attr) noexcept {
        auto location = (addrs.vAddr.addr >> 12) & 0x3FF;
        data[location] = (addrs.pAddr.addr & (~0x3FF)) | attr;
    }

    bool unmap(VirtualAddress virtualAddr, PhysicalAddress& physicalAddr) noexcept {
        auto location = (virtualAddr.addr >> 12) & 0x3FF;
        if (data[location] & PTE_Present) {
            physicalAddr.addr = data[location] & (~0x3FF);
            data[location] = 0;
            return true;
        } else {
            return false;
        }
    }

    PhysicalAddress alloc(VirtualAddress virtualAddr, uint16_t attr) noexcept {
        auto physicalAddr = allocator::physicalFrameAlloc();
        map({.pAddr = physicalAddr, .vAddr = virtualAddr}, attr);
        return physicalAddr;
    }

    void free() noexcept {
        for (size_t i = 0; i < 1024; i++) {
            if (!(data[i] & PTE_Present)) {
                continue;
            }
            allocator::physicalFrameRelease(PhysicalAddress{data[i] & (~0x3FF)});
        }
    }
};

}  // namespace nyan::paging
