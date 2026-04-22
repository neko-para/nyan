#include <nyan/syscall.h>
#include <string.h>

#include "allocator/load.hpp"
#include "arch/io.hpp"
#include "boot/entry.hpp"
#include "console/entry.hpp"
#include "gdt/load.hpp"
#include "interrupt/load.hpp"
#include "keyboard/load.hpp"
#include "lib/format.hpp"
#include "paging/kernel.hpp"
#include "task/task.hpp"
#include "task/tcb.hpp"
#include "timer/load.hpp"

#include "test.hpp"

extern "C" void __libc_init_array();
extern uint8_t _end;

namespace nyan {

extern "C" void kmain(boot::BootInfo* info) {
    arch::enableSse();
    paging::clearIdentityPaging();

    __libc_init_array();

    info = paging::PhysicalAddress(info).kernelToVirtual().as<boot::BootInfo>();
    auto mmap_count = info->mmap_length / sizeof(boot::MMapEntry);
    info->mmap_addr = paging::PhysicalAddress(info->mmap_addr).kernelToVirtual().as<boot::MMapEntry>();

    for (size_t i = 0; i < mmap_count; i++) {
        auto& entry = info->mmap_addr[i];
        if (entry.type != boot::MMT_Available) {
            continue;
        }
        if (entry.addr_lo == 0) {
            continue;
        }

        uint32_t upper = entry.addr_lo + entry.len_lo;
        allocator::load(upper);
        break;
    }

    console::load();

    gdt::load();
    interrupt::load();

    timer::load();
    keyboard::load();

    task::load();

    console::loadDeamons();

    lib::__list::List<Tag, true> lst;
    for (int i = 0; i < 10; i++) {
        auto item = allocator::allocAs<Item>();
        item->value = i;
        lst.push_back(item);
    }
    auto p2 = std::find_if(lst.cbegin(), lst.cend(), [](const auto& item) { return item.value == 2; });
    auto p5 = std::find_if(lst.cbegin(), lst.cend(), [](const auto& item) { return item.value == 5; });
    lst.erase(p2, p5);
    for (const auto& item : lst) {
        arch::kprint("{p} {}\n", &item, item.value);
    }

    arch::kprint("kernel end {#010x}\n", paging::VirtualAddress(&_end).kernelToPhysical().addr);
    task::yield();

    for (;;) {
        auto ret = syscall::waitpid(-1, 0, 0);
        if (ret == -SYS_ECHILD) {
            task::currentTask->wait.wait(task::BlockReason::BR_WaitTask);
        }
    }
}

}  // namespace nyan
