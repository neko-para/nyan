#include <nyan/syscall.h>
#include <string.h>

#include "allocator/load.hpp"
#include "arch/io.hpp"
#include "boot/entry.hpp"
#include "data/embed.hpp"
#include "gdt/load.hpp"
#include "interrupt/load.hpp"
#include "keyboard/load.hpp"
#include "lib/format.hpp"
#include "paging/kernel.hpp"
#include "task/task.hpp"
#include "task/tcb.hpp"
#include "timer/load.hpp"
#include "vga/cursor.hpp"
#include "vga/print.hpp"

extern "C" void __libc_init_array();
extern uint8_t _end;

namespace nyan {

int logic(void*) {
    arch::kprint("kernel end {#010x}\n", paging::VirtualAddress(&_end).kernelToPhysical().addr);

    const char* argv[] = {0};
    auto tcb = task::createElfTask(data::programs[0].data, data::programs[0].size, argv);
    auto pid = task::addTask(tcb);

    syscall::waitpid(pid, 0, 0);
    return 0;
}

extern "C" void kmain(boot::BootInfo* info) {
    arch::enableSse();
    paging::clearIdentityPaging();

    vga::clear();
    vga::showCursor();
    vga::flushCursor();

    __libc_init_array();

    gdt::load();
    interrupt::load();

    timer::load();
    keyboard::load();

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

    task::load();
    task::addTask(task::createTask(logic));
    task::initYield();

    arch::sti();
    vga::print("all tasks finished.\n");
    arch::kprint("all tasks finished.\n");
    for (int i = 0; i < task::MaxTaskCount; i++) {
        if (task::allTasks[i]) {
            task::allTasks[i]->dump();
            if (task::allTasks[i]->state == task::State::S_Exited) {
                task::freeTask(i, nullptr);
            }
        }
    }

    for (int i = 0; i < task::MaxTaskCount; i++) {
        if (task::allTasks[i]) {
            task::allTasks[i]->dump();
        }
    }

    for (;;) {
        arch::hlt();
    }
}

}  // namespace nyan
