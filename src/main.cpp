#include <stdio.h>
#include <string.h>

#include "allocator/load.hpp"
#include "boot/entry.hpp"
#include "data/embed.hpp"
#include "elf/entry.hpp"
#include "gdt/load.hpp"
#include "interrupt/load.hpp"
#include "keyboard/load.hpp"
#include "keyboard/message.hpp"
#include "lib/containers.hpp"
#include "lib/format.hpp"
#include "lib/function.hpp"
#include "paging/kernel.hpp"
#include "syscall/entry.hpp"
#include "task/task.hpp"
#include "timer/load.hpp"
#include "vga/cursor.hpp"
#include "vga/print.hpp"

extern "C" void __libc_init_array();
extern uint8_t _end;

namespace nyan {

extern "C" void kmain(boot::BootInfo* info) {
    arch::enableSse();
    paging::clearIdentityPaging();

    vga::clear();
    vga::showCursor();

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

    arch::sti();

    vga::print("kernel end {#010x}\n", paging::VirtualAddress(&_end).kernelToPhysical().addr);

    task::load();

    pid_t pid;
    {
        auto tcb =
            task::createElfTask(&_binary_prog_bin_start[0], &_binary_prog_bin_end[0] - &_binary_prog_bin_start[0]);
        pid = task::addTask(tcb);
    }

    task::addTask(task::createTask(
        +[](void* param) {
            syscall::waitpid(reinterpret_cast<pid_t>(param), 0, 0);
            return 0;
        },
        reinterpret_cast<void*>(pid)));

    task::initYield();

    vga::print("all tasks finished.\n");

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
