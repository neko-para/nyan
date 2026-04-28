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
        auto begin = paging::VirtualAddress{addr};
        if (begin.thisPage() != begin) {
            return reinterpret_cast<void*>(-SYS_EINVAL);
        }
        auto place = task::currentTask->vmSpace.find_free(length, begin);
        if (*place != begin) {
            arch::kprint("mmap2 fixed failed, want {#10x}\n", begin.addr);
            task::currentTask->vmSpace.dump();
            return reinterpret_cast<void*>(-SYS_ENOMEM);
        }
        if (!task::currentTask->vmSpace.insert(paging::VMA{
                *place,
                *place + length,
                static_cast<uint32_t>(flags),
                static_cast<uint32_t>(prot),
                "mmap",
            })) {
            return reinterpret_cast<void*>(-SYS_ENOMEM);
        }
        return place->as<void>();
    }
    if (flags & MAP_PRIVATE) {
        if (length == 0) {
            return reinterpret_cast<void*>(-SYS_EINVAL);
        }
        length = (length + 0xFFF) & (~0xFFF);
        auto begin = paging::VirtualAddress{addr};
        auto place = task::currentTask->vmSpace.find_free(length);
        if (!place) {
            return reinterpret_cast<void*>(-SYS_ENOMEM);
        }
        if (!task::currentTask->vmSpace.insert(paging::VMA{*place, *place + length, static_cast<uint32_t>(flags),
                                                           static_cast<uint32_t>(prot), "mmap"})) {
            return reinterpret_cast<void*>(-SYS_ENOMEM);
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
