#include <stdarg.h>
#include <__verbose_abort>
#include <new>
#include <string>

#include "../allocator/alloc.hpp"
#include "../arch/debug.hpp"

_LIBCPP_BEGIN_NAMESPACE_STD

[[noreturn]] void __libcpp_verbose_abort(const char* fmt, ...) noexcept {
    va_list lst;
    va_start(lst, fmt);
    while (*fmt) {
        switch (*fmt) {
            case '%':
                switch (fmt[1]) {
                    case '%':
                        nyan::arch::kput('%');
                        break;
                    case 's': {
                        auto str = va_arg(lst, const char*);
                        nyan::arch::kputs(str ? str : "");
                        break;
                    }
                    case 'i':
                    case 'd': {
                        char cache[12];
                        char* ptr = cache;
                        int val = va_arg(lst, int);
                        if (val < 0) {
                            nyan::arch::kput('-');
                            val = -val;
                        }
                        do {
                            *ptr++ = (val % 10) + '0';
                            val /= 10;
                        } while (val > 0);
                        while (ptr != cache) {
                            nyan::arch::kput(*--ptr);
                        }
                        break;
                    }
                    default:
                        va_arg(lst, size_t);
                }
                fmt += 2;
                break;
            default:
                nyan::arch::kput(*fmt);
                fmt += 1;
        }
    }
    nyan::arch::kfatal();
}

_LIBCPP_END_NAMESPACE_STD

void* operator new(size_t size) {
    return nyan::allocator::slabAlloc(size, size);
}

void* operator new[](size_t size) {
    return nyan::allocator::slabAlloc(size, size);
}

void* operator new(size_t size, std::align_val_t align) {
    return nyan::allocator::slabAlloc(size, static_cast<size_t>(align));
}

void operator delete(void* ptr) noexcept {
    nyan::allocator::slabFree(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    nyan::allocator::slabFree(ptr);
}

void operator delete(void* ptr, size_t, std::align_val_t) noexcept {
    nyan::allocator::slabFree(ptr);
}

void operator delete[](void* ptr) noexcept {
    nyan::allocator::slabFree(ptr);
}
