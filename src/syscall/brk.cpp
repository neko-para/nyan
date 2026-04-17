#include <nyan/syscall.h>

#include "../paging/address.hpp"
#include "../paging/directory.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

static void allocPageAt(paging::UserDirectory& pageDir, uint32_t addr) {
    auto virtualAddr = paging::VirtualAddress{addr};
    auto tableLocation = virtualAddr.tableLoc();
    pageDir.ensure(tableLocation, paging::PDE_Present | paging::PDE_ReadWrite | paging::PDE_User);
    auto physicalAddr = pageDir.with(tableLocation, [&](paging::Table* table) {
        return table->alloc(virtualAddr, paging::PTE_Present | paging::PTE_ReadWrite | paging::PTE_User);
    });

    paging::MapperGuard mapper(physicalAddr);
    auto frame = mapper.as<uint8_t>();
    std::fill_n(frame, 0x1000, 0);
}

void* brk(const void* addr) {
    auto vAddr = paging::VirtualAddress{addr};
    if (!vAddr || vAddr.addr < task::currentTask->brkAddr.addr) {
        return task::currentTask->brkAddr.as<void>();
    }
    auto currentPage = (task::currentTask->brkAddr.addr - 1) & (~0xFFF);
    auto wantPage = (vAddr.addr - 1) & (~0xFFF);

    if (currentPage != wantPage) {
        auto pageDir = paging::UserDirectory::from(task::currentTask->cr3);
        while (currentPage != wantPage) {
            currentPage += 0x1000;
            allocPageAt(pageDir, currentPage);
        }
    }
    return vAddr.as<void>();
}

}  // namespace nyan::syscall
