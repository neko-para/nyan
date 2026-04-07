#include <string.h>

#include "setup/paging.hpp"
#include "setup/sse.hpp"
#include "vga/print.hpp"

extern "C" void __libc_init_array();

namespace nyan {

extern "C" void kmain() {
    setup::clearIdentityPaging();
    setup::enableSse();

    vga::clear();

    __libc_init_array();

    char* msg = new char[20];
    strcpy(msg, "Hello world!");

    vga::puts(msg);
}

}  // namespace nyan
