#include <stddef.h>
#include <stdint.h>

extern "C" {

int memcmp(const void* lhs, const void* rhs, size_t size) {
    auto plhs = reinterpret_cast<const uint8_t*>(lhs);
    auto prhs = reinterpret_cast<const uint8_t*>(rhs);
    while (size--) {
        if (*plhs != *prhs) {
            return *plhs - *prhs;
        } else {
            plhs++;
            prhs++;
        }
    }
    return 0;
}

void* memcpy(void* dst, const void* src, size_t size) {
    auto pdst = reinterpret_cast<uint8_t*>(dst);
    auto psrc = reinterpret_cast<const uint8_t*>(src);
    while (size--) {
        *pdst++ = *psrc++;
    }
    return dst;
}

void* memmove(void* dst, const void* src, size_t size) {
    auto pdst = reinterpret_cast<uint8_t*>(dst);
    auto psrc = reinterpret_cast<const uint8_t*>(src);
    if (pdst < psrc) {
        while (size--) {
            *pdst++ = *psrc++;
        }
    } else if (pdst > psrc) {
        pdst += size;
        psrc += size;
        while (size--) {
            *--pdst = *--psrc;
        }
    }
    return dst;
}

void* memset(void* dst, int value, size_t size) {
    auto pdst = reinterpret_cast<uint8_t*>(dst);
    while (size--) {
        *pdst++ = value;
    }
    return dst;
}

void* memchr(const void* src, int ch, size_t size) {
    auto psrc = static_cast<const uint8_t*>(src);
    uint8_t pat = static_cast<uint8_t>(ch);
    while (size--) {
        if (*psrc == pat) {
            return const_cast<uint8_t*>(psrc);
        }
        psrc++;
    }
    return 0;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

extern uintptr_t __preinit_array_start[];
extern uintptr_t __preinit_array_end[];
extern uintptr_t __init_array_start[];
extern uintptr_t __init_array_end[];

void __libc_init_array(void) {
    using InitCallback = void(void);

    size_t preinit_array_size = __preinit_array_end - __preinit_array_start;
    for (size_t i = 0; i < preinit_array_size; ++i)
        reinterpret_cast<InitCallback*>(__preinit_array_start[i])();
    size_t init_array_size = __init_array_end - __init_array_start;
    for (size_t i = 0; i < init_array_size; ++i)
        reinterpret_cast<InitCallback*>(__init_array_start[i])();
}

}  //