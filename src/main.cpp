#include <stdio.h>
#include <string.h>

#include "arch/io.hpp"
#include "boot/entry.hpp"
#include "gdt/load.hpp"
#include "interrupt/load.hpp"
#include "keyboard/load.hpp"
#include "paging/convert.hpp"
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
        printf("%08lx ~ %08lx, %lu, %lu\n", entry.addr_lo, entry.addr_lo + entry.len_lo, entry.len_lo, entry.type);
    }

    char* msg = new char[20];
    strcpy(msg, "Hello world!");

    vga::puts(msg);

    for (;;) {
        arch::hlt();
    }
}

}  // namespace nyan
