#include <nyan/syscall.h>

#include "../paging/address.hpp"
#include "../paging/directory.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

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
            pageDir.alloc(paging::VirtualAddress{currentPage}, true);
        }
    }
    return vAddr.as<void>();
}

}  // namespace nyan::syscall
