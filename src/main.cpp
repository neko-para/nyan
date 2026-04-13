#include <stdio.h>
#include <string.h>

#include "allocator/load.hpp"
#include "boot/entry.hpp"
#include "gdt/load.hpp"
#include "interrupt/load.hpp"
#include "keyboard/load.hpp"
#include "keyboard/message.hpp"
#include "lib/containers.hpp"
#include "lib/format.hpp"
#include "lib/function.hpp"
#include "paging/convert.hpp"
#include "paging/kernel.hpp"
#include "task/task.hpp"
#include "timer/load.hpp"
#include "vga/cursor.hpp"
#include "vga/print.hpp"

extern "C" void __libc_init_array();
extern uint8_t _end;

namespace nyan {

static void sleep(uint64_t ms) {
    timespec spec;
    spec.tv_sec = ms / 1000;
    spec.tv_nsec = (ms % 1000) * 1000000;
    asm volatile("int $0x80;" ::"a"(162), "b"(&spec), "c"(0));
}

static void subTask() {
    task::pid_t pid = 0;
    asm volatile("int $0x80;" : "=a"(pid) : "a"(20));
    asm volatile("int $0x80;" ::"a"(4), "b"(1), "c"("hello!"), "d"(6));
    // vga::print("task {}: start\n", pid);

    sleep((pid - 14) * 500);
    // vga::print("task {}: awake\n", pid);

    asm volatile("int $0x80;" ::"a"(1), "b"(123));
}

static int jumpTask(void*) {
    task::jumpRing3(subTask);
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

    vga::print("kernel end {010p}\n", paging::virtualToPhysical(&_end));

    task::load();

    for (int i = 0; i < 5; i++) {
        task::runTask(jumpTask);
    }
    task::initYield();

    lib::string str = "all tasks finished.\n";
    vga::puts(str.c_str());

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
