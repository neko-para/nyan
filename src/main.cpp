#include "setup/paging.hpp"
#include "vga/print.hpp"

namespace nyan {

extern "C" void kmain() {
    setup::clearIdentityPaging();

    vga::clear();
    vga::puts("Hello world!");
}

}  // namespace nyan
