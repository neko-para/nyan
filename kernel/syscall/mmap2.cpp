#include <nyan/syscall.h>
#include <sys/mman.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

void* mmap2(void* addr, size_t length, int prot, int flags, int, uint32_t) {
    if (flags & MAP_FILE) {
        return reinterpret_cast<void*>(-SYS_ENOSYS);
    }
    if (flags & MAP_FIXED) {
        if (length == 0) {
            return reinterpret_cast<void*>(-SYS_EINVAL);
        }
        length = (length + 0xFFF) & (~0xFFF);
        auto begin = paging::VirtualAddress{addr};
        if (begin.thisPage() != begin) {
            return reinterpret_cast<void*>(-SYS_EINVAL);
        }
        auto pageDir = paging::UserDirectory::from(task::currentTask->cr3);
        task::currentTask->vmSpace.erase(begin, length, pageDir);
        auto place = task::currentTask->vmSpace.find_free(length, begin);
        if (!place || *place != begin) {
            arch::kprint("mmap2 fixed failed, want {#10x}\n", begin.addr);
            task::currentTask->vmSpace.dump();
            return reinterpret_cast<void*>(-SYS_ENOMEM);
        }
        auto vma = paging::VMA{
            *place, *place + length, static_cast<uint32_t>(flags), static_cast<uint32_t>(prot), "mmap",
        };
        if (!task::currentTask->vmSpace.insert(vma)) {
            return reinterpret_cast<void*>(-SYS_ENOMEM);
        } else {
            vma.alloc(pageDir);
        }
        return place->as<void>();
    }
    if (flags & MAP_PRIVATE) {
        if (length == 0) {
            return reinterpret_cast<void*>(-SYS_EINVAL);
        }
        length = (length + 0xFFF) & (~0xFFF);
        auto begin = paging::VirtualAddress{addr};
        auto place = task::currentTask->vmSpace.find_free(length, begin);
        if (!place) {
            return reinterpret_cast<void*>(-SYS_ENOMEM);
        }
        auto pageDir = paging::UserDirectory::from(task::currentTask->cr3);
        auto vma = paging::VMA{
            *place, *place + length, static_cast<uint32_t>(flags), static_cast<uint32_t>(prot), "mmap",
        };
        if (!task::currentTask->vmSpace.insert(vma)) {
            return reinterpret_cast<void*>(-SYS_ENOMEM);
        } else {
            vma.alloc(pageDir);
        }
        return place->as<void>();
    } else if (flags & MAP_SHARED) {
        return reinterpret_cast<void*>(-SYS_ENOSYS);
    } else {
        return reinterpret_cast<void*>(-SYS_EINVAL);
    }
    return 0;
}

}  // namespace nyan::syscall
