#include <nyan/syscall.h>

#include "../paging/address.hpp"
#include "../paging/directory.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

// TODO: 支持收缩
void* brk(const void* addr) {
    auto vAddr = paging::VirtualAddress{addr};
    if (!vAddr || vAddr <= task::__scheduler->__current->brkAddr) {
        return task::__scheduler->__current->brkAddr.as<void>();
    }
    auto currentPage = (task::__scheduler->__current->brkAddr - 1).thisPage();
    auto wantPage = (vAddr - 1).thisPage();

    if (currentPage != wantPage) {
        auto pageDir = paging::UserDirectory::from(task::__scheduler->__current->cr3);
        while (currentPage != wantPage) {
            currentPage = currentPage.nextPage();
            pageDir.alloc(currentPage, true);
        }
    }

    auto& vmSpace = task::__scheduler->__current->vmSpace;

    if (auto vma = vmSpace.find_name("brk"); vma == vmSpace.__addrs.end()) {
        vmSpace.insert(paging::VMA{
            task::__scheduler->__current->brkBase,
            wantPage.nextPage(),
            MAP_PRIVATE | MAP_ANONYMOUS,
            PROT_READ | PROT_WRITE,
            "brk",
        });
    } else {
        auto next = std::next(vma);
        if (next != vmSpace.__addrs.end()) {
            if (wantPage.nextPage() > next->__begin) {
                return reinterpret_cast<void*>(-SYS_ENOMEM);
            }
        }
        vma->__end = wantPage.nextPage();
    }

    task::__scheduler->__current->brkAddr = vAddr;
    return vAddr.as<void>();
}

}  // namespace nyan::syscall
