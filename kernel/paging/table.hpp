#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <algorithm>

#include "../allocator/alloc.hpp"
#include "../arch/print.hpp"
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
    PhysicalAddress at(uint16_t location) const noexcept { return PhysicalAddress{data[location] & (~0xFFF)}; }
    uint16_t attr(uint16_t location) const noexcept { return data[location] & 0xFFF; }
    bool isPresent(uint16_t location) const noexcept { return data[location] & PTE_Present; }

    void map(VirtualAddress vAddr, PhysicalAddress pAddr, uint16_t attr) noexcept {
        auto location = (vAddr.addr >> 12) & 0x3FF;
        data[location] = pAddr.thisPage().addr | attr;
    }

    bool unmap(VirtualAddress virtualAddr, PhysicalAddress& physicalAddr) noexcept {
        auto location = (virtualAddr.addr >> 12) & 0x3FF;
        if (data[location] & PTE_Present) {
            physicalAddr.addr = data[location] & (~0xFFF);
            data[location] = 0;
            return true;
        } else {
            return false;
        }
    }

    PhysicalAddress alloc(VirtualAddress virtualAddr, uint16_t attr) noexcept {
        auto physicalAddr = allocator::physicalFrameAlloc();
        map(virtualAddr, physicalAddr, attr);
        return physicalAddr;
    }

    void freeDangling() noexcept {
        for (size_t i = 0; i < 1024; i++) {
            if (!(data[i] & PTE_Present)) {
                continue;
            }
            arch::kprint("dangling page detected, {} - {}\n", i, at(i).addr);
            allocator::physicalFrameRelease(at(i));
        }
    }

    void freePage(VirtualAddress addr) {
        auto loc = addr.tablePageLoc();
        if (isPresent(loc)) {
            allocator::physicalFrameRelease(at(loc));
            data[loc] = 0;
            addr.invlpg();
        }
    }
};

}  // namespace nyan::paging
