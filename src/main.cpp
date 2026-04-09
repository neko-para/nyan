#include <stdio.h>
#include <string.h>

#include "allocator/pool.hpp"
#include "arch/io.hpp"
#include "boot/entry.hpp"
#include "gdt/load.hpp"
#include "interrupt/load.hpp"
#include "keyboard/load.hpp"
#include "paging/convert.hpp"
#include "paging/kernel.hpp"
#include "setup/paging.hpp"
#include "setup/sse.hpp"
#include "timer/load.hpp"
#include "vga/print.hpp"

extern "C" void __libc_init_array();

namespace nyan {

extern "C" void kmain(boot::BootInfo* info) {
    setup::clearIdentityPaging();
    setup::enableSse();

    vga::clear();

    __libc_init_array();

    gdt::load();
    interrupt::load();

    timer::load();
    keyboard::load();

    arch::sti();

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
        allocator::poolManager = new allocator::PoolManager(upper - allocator::base);
        break;
    }

    printf("%p %p\n", &paging::kernelPageDirectory, paging::kernelPageTable);

    char* msg = new char[20];
    strcpy(msg, "Hello world!");

    vga::puts(msg);

    for (;;) {
        arch::hlt();
    }
}

}  // namespace nyan
