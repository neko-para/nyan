#include <nyan/syscall.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

int munmap(void* addr, size_t length) {
    auto begin = paging::VirtualAddress{addr};
    if (begin.thisPage() != begin) {
        return -SYS_EINVAL;
    }
    length = (length + 0xFFF) & (~0xFFF);
    auto pageDir = paging::UserDirectory::from(task::currentTask->cr3);
    if (task::currentTask->vmSpace.erase(paging::VirtualAddress{addr}, length, pageDir)) {
        return 0;
    } else {
        return -SYS_EINVAL;
    }
}

}  // namespace nyan::syscall