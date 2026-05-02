#include "table.hpp"

#include "../allocator/mod.hpp"
#include "../arch/print.hpp"

namespace nyan::paging {

PhysicalAddress Table::alloc(VirtualAddress virtualAddr, uint16_t attr) noexcept {
    auto physicalAddr = allocator::physicalFrameAlloc();
    map(virtualAddr, physicalAddr, attr);
    return physicalAddr;
}

void Table::freeDangling() noexcept {
    for (size_t i = 0; i < 1024; i++) {
        if (!(data[i] & PTE_Present)) {
            continue;
        }
        arch::kprint("dangling page detected, {} - {}\n", i, at(i).addr);
        allocator::physicalFrameRelease(at(i));
    }
}

void Table::freePage(VirtualAddress addr) noexcept {
    auto loc = addr.tablePageLoc();
    if (isPresent(loc)) {
        allocator::physicalFrameRelease(at(loc));
        data[loc] = 0;
        addr.invlpg();
    }
}

}  // namespace nyan::paging
