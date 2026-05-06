#include <nyan/syscall.h>
#include <sys/mman.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

void* mmap2(void* addr, size_t length, int prot, int flags, int, uint32_t) {
    if (flags & MAP_FILE) {
        return SYS_ENOSYS;
    }
    if (flags & MAP_FIXED) {
        if (length == 0) {
            return SYS_EINVAL;
        }
        length = (length + 0xFFF) & (~0xFFF);
        auto begin = paging::VirtualAddress{addr};
        if (begin.thisPage() != begin) {
            return SYS_EINVAL;
        }
        auto pageDir = paging::UserDirectory::from(task::__scheduler->__current->cr3);
        task::__scheduler->__current->vmSpace.erase(begin, length, pageDir);
        auto place = task::__scheduler->__current->vmSpace.find_free(length, begin);
        if (!place || *place != begin) {
            arch::kprint("mmap2 fixed failed, want {#10x}\n", begin.addr);
            task::__scheduler->__current->vmSpace.dump();
            return SYS_ENOMEM;
        }
        auto vma = paging::VMA{
            *place, *place + length, static_cast<uint32_t>(flags), static_cast<uint32_t>(prot), "mmap",
        };
        if (!task::__scheduler->__current->vmSpace.insert(vma)) {
            return SYS_ENOMEM;
        } else {
            vma.alloc(pageDir);
        }
        return place->as<void>();
    }
    if (flags & MAP_PRIVATE) {
        if (length == 0) {
            return SYS_EINVAL;
        }
        length = (length + 0xFFF) & (~0xFFF);
        auto begin = paging::VirtualAddress{addr};
        auto place = task::__scheduler->__current->vmSpace.find_free(length, begin);
        if (!place) {
            return SYS_ENOMEM;
        }
        auto pageDir = paging::UserDirectory::from(task::__scheduler->__current->cr3);
        auto vma = paging::VMA{
            *place, *place + length, static_cast<uint32_t>(flags), static_cast<uint32_t>(prot), "mmap",
        };
        if (!task::__scheduler->__current->vmSpace.insert(vma)) {
            return SYS_ENOMEM;
        } else {
            vma.alloc(pageDir);
        }
        return place->as<void>();
    } else if (flags & MAP_SHARED) {
        return SYS_ENOSYS;
    } else {
        return SYS_EINVAL;
    }
    return 0;
}

}  // namespace nyan::syscall
