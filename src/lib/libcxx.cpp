#include <stdio.h>
#include <stdlib.h>
#include <__verbose_abort>
#include <new>

#include "../arch/io.hpp"

_LIBCPP_BEGIN_NAMESPACE_STD

[[__noreturn__]]
_LIBCPP_ATTRIBUTE_FORMAT(__printf__, 1, 2) void __libcpp_verbose_abort(const char* __format, ...) noexcept {
    static char buf[256];
    va_list lst;
    va_start(lst, __format);
    vsnprintf(buf, 256, __format, lst);
    va_end(lst);

    nyan::arch::kputs(buf);
    nyan::arch::cli();
    while (true) {
        nyan::arch::hlt();
    }
}

_LIBCPP_END_NAMESPACE_STD

void* operator new(size_t size) {
    return malloc(size);
}

void* operator new[](size_t size) {
    return malloc(size);
}

void operator delete(void* ptr) noexcept {
    return free(ptr);
}

void operator delete[](void* ptr) noexcept {
    return free(ptr);
}
