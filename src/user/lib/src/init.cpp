#include "stdio_impl.h"

extern "C" void __init_libc() {
    __init_stdio();
}
