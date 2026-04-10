#include <stdio.h>
#include <string.h>

#include "allocator/load.hpp"
#include "allocator/slab.hpp"
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

void idleTask(void*) {
    while (true) {
        arch::hlt();
        task::yield();
    }
}

task::TaskControlBlock* tasks[5];

void subTask(void* param) {
    uint32_t id = reinterpret_cast<uint32_t>(param);
    printf("task %u: start\n", id);
    task::sleep((id + 1) * 1000);
    printf("task %u: awake\n", id);
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

    arch::sti();

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

    char* msg = (char*)allocator::slabManager->alloc(20);
    strcpy(msg, "Hello world!\n");
    vga::puts(msg);
    allocator::slabManager->free(msg);

    auto idle = task::createTask(idleTask, 0);
    task::addTask(idle);

    for (int i = 0; i < 5; i++) {
        tasks[i] = task::createTask(subTask, reinterpret_cast<void*>(i));
        task::addTask(tasks[i]);
    }
    task::initYield();

    vga::puts("all tasks finished.\n");

    for (int i = 0; i < 5; i++) {
        printf("task %d: status %u\n", i, tasks[i]->state);
    }

    for (;;) {
        arch::hlt();
    }
}

}  // namespace nyan
