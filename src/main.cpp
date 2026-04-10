#include <stdio.h>
#include <string.h>

#include "allocator/load.hpp"
#include "arch/io.hpp"
#include "boot/entry.hpp"
#include "gdt/load.hpp"
#include "interrupt/load.hpp"
#include "keyboard/load.hpp"
#include "keyboard/message.hpp"
#include "paging/convert.hpp"
#include "paging/kernel.hpp"
#include "task/task.hpp"
#include "timer/load.hpp"
#include "vga/cursor.hpp"
#include "vga/print.hpp"

extern "C" void __libc_init_array();
extern uint8_t _end;

namespace nyan {

static int subTask(void*) {
    auto pid = task::currentTask->pid;
    printf("task %u: start\n", pid);
    task::sleep((pid - 14) * 500);
    printf("task %u: awake\n", pid);
    return 0;
}

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

    keyboard::keyboardCallback = +[](const keyboard::Message& msg) {
        if (!(msg.flag & keyboard::F_Release)) {
            vga::putc(msg.ch);
            if (msg.flag & keyboard::F_Ctrl && msg.code == keyboard::SC_C) {
                arch::qemuQuit();
            }
        }
    };

    printf("kernel end %p\n", paging::virtualToPhysical(&_end));

    info = paging::physicalToVirtual(info);
    auto mmap_count = info->mmap_length / sizeof(boot::MMapEntry);
    info->mmap_addr = paging::physicalToVirtual(info->mmap_addr);

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

    task::load();

    for (int i = 0; i < 5; i++) {
        task::runTask(subTask);
    }
    task::initYield();

    vga::puts("all tasks finished.\n");

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
