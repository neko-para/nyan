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

void subTask(void* param) {
    printf("inside task %p!\n", param);
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

    auto tcb1 = task::createTask(subTask, reinterpret_cast<void*>(1));
    auto tcb2 = task::createTask(subTask, reinterpret_cast<void*>(2));
    auto tcb3 = task::createTask(subTask, reinterpret_cast<void*>(3));
    task::addTask(tcb1);
    task::addTask(tcb2);
    task::addTask(tcb3);
    task::initYield();

    vga::puts("all tasks finished.\n");

    for (;;) {
        arch::hlt();
    }
}

}  // namespace nyan
