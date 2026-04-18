#include <nyan/syscall.h>

#include "../paging/address.hpp"
#include "../paging/directory.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

void* brk(const void* addr) {
    auto vAddr = paging::VirtualAddress{addr};
    if (!vAddr || vAddr < task::currentTask->brkAddr) {
        return task::currentTask->brkAddr.as<void>();
    }
    auto currentPage = (task::currentTask->brkAddr - 1).thisPage();
    auto wantPage = (vAddr - 1).thisPage();

    if (currentPage != wantPage) {
        auto pageDir = paging::UserDirectory::from(task::currentTask->cr3);
        while (currentPage != wantPage) {
            currentPage = currentPage.nextPage();
            pageDir.alloc(currentPage, true);
        }
    }
    task::currentTask->brkAddr = vAddr;
    return vAddr.as<void>();
}

}  // namespace nyan::syscall
