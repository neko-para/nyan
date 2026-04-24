#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <nyan/syscall.h>
#include <stdlib.h>
#include <string.h>

#include "stdlib_impl.h"
#include "utils.hpp"

using namespace nyan::user;

namespace {

template <typename T>
struct limits {};

template <>
struct limits<long> {
    constexpr static bool sign = true;
    constexpr static long min = LONG_MIN;
    constexpr static long max = LONG_MAX;
};

template <>
struct limits<long long> {
    constexpr static bool sign = true;
    constexpr static long long min = LLONG_MIN;
    constexpr static long long max = LLONG_MAX;
};

template <>
struct limits<unsigned long> {
    constexpr static bool sign = false;
    constexpr static unsigned long min = 0;
    constexpr static unsigned long max = ULONG_MAX;
};

template <>
struct limits<unsigned long long> {
    constexpr static bool sign = false;
    constexpr static unsigned long long min = 0;
    constexpr static unsigned long long max = ULLONG_MAX;
};

bool seekOctPrefix(const char* str) {
    // 0? 也可以
    return str[0] == '0';
}

bool seekHexPrefix(const char* str) {
    return str[0] == '0' && (str[1] == 'X' || str[1] == 'x') && isxdigit(str[2]);
}

template <typename T>
int strto(T& result, const char* str, char** end, int base) {
    if (base < 0 || base == 1 || base > 36) {
        result = 0;
        if (end) {
            *end = const_cast<char*>(str);
        }
        return EINVAL;
    }
    if (!str) {
        result = 0;
        if (end) {
            *end = const_cast<char*>(str);
        }
        return EINVAL;
    }
    T value = 0;
    auto beg = str;
    bool negative = false;
    bool valid = false;
    bool overflow = false;
    while (isspace(*str)) {
        str++;
    }
    if (*str == '-') {
        str++;
        negative = true;
    } else if (*str == '+') {
        str++;
        negative = false;
    }
    if (base == 0) {
        if (seekHexPrefix(str)) {
            str += 2;
            base = 16;
        } else if (seekOctPrefix(str)) {
            str += 1;
            base = 8;
        } else {
            base = 10;
        }
    } else if (base == 8) {
        if (seekOctPrefix(str)) {
            str += 1;
        }
    } else if (base == 16) {
        if (seekHexPrefix(str)) {
            str += 2;
        }
    }
    while (isalnum(*str)) {
        auto ch = tolower(*str);
        int val;
        if (isdigit(ch)) {
            val = ch - '0';
        } else {
            val = ch - 'a' + 10;
        }
        if (val >= base) {
            break;
        }
        valid = true;
        str++;
        if (overflow) {
            continue;
        }
        if (__builtin_mul_overflow(value, base, &value)) {
            overflow = true;
            continue;
        }
        if constexpr (limits<T>::sign) {
            if (__builtin_add_overflow(value, -val, &value)) {
                overflow = true;
            }
        } else {
            if (__builtin_add_overflow(value, val, &value)) {
                overflow = true;
            }
        }
    }
    if (end) {
        if (valid) {
            *end = const_cast<char*>(str);
        } else {
            *end = const_cast<char*>(beg);
        }
    }
    if constexpr (limits<T>::sign) {
        if (!overflow && !negative && value == limits<T>::min) {
            overflow = true;
        }
        if (overflow) {
            result = negative ? limits<T>::min : limits<T>::max;
            return ERANGE;
        }
        result = negative ? value : -value;
    } else {
        if (overflow) {
            result = limits<T>::max;
            return ERANGE;
        }
        result = negative ? -value : value;
    }
    return 0;
}

}  // namespace

namespace nyan::user {

static void __slab_list_remove(SlabHeader*& head, SlabHeader* target) {
    if (head == target) {
        head = target->__next;
        target->__next = nullptr;
        return;
    }
    auto prev = head;
    while (prev && prev->__next != target) {
        prev = prev->__next;
    }
    if (prev) {
        prev->__next = target->__next;
        target->__next = nullptr;
    }
}

SlabHeader::SlabHeader(size_t size, SlabCache* cache) : __chunk_size(size), __used_count(0), __cache(cache) {
    auto self = reinterpret_cast<uint32_t>(this);
    auto dlt = 1 << size;
    auto cnt = (4096 >> size) - 1;

    auto addr = self + 4096 - dlt;
    while (cnt--) {
        auto chunk = reinterpret_cast<SlabChunk*>(addr);
        chunk->__next = __first_chunk;
        __first_chunk = chunk;
        addr -= dlt;
    }
}

void* SlabHeader::alloc() noexcept {
    __used_count += 1;
    auto ret = __first_chunk;
    __first_chunk = __first_chunk->__next;
    return ret;
}

void SlabHeader::free(void* addr) noexcept {
    auto chunk = static_cast<SlabChunk*>(addr);
    __used_count -= 1;
    chunk->__next = __first_chunk;
    __first_chunk = chunk;
}

void* SlabManager::alloc(size_t size) noexcept {
    size_t chunk_size;
    auto cache = findSuitableCache(size, chunk_size);
    if (!cache) {
        return nullptr;
    }

    if (cache->__used_slabs) {
        auto ret = cache->__used_slabs->alloc();
        if (cache->__used_slabs->full()) {
            auto slab = cache->__used_slabs;
            cache->__used_slabs = cache->__used_slabs->__next;
            slab->__next = cache->__full_slabs;
            cache->__full_slabs = slab;
        }
        return ret;
    } else {
        auto page = __allocPage();
        auto slab = new (page) SlabHeader(chunk_size, cache);
        auto ret = slab->alloc();
        slab->__next = cache->__used_slabs;
        cache->__used_slabs = slab;
        return ret;
    }
}

void SlabManager::free(void* addr) noexcept {
    auto chunk = SlabChunk::fromAddr(addr);
    auto slab = SlabHeader::fromAddr(addr);
    auto cache = slab->__cache;
    slab->free(chunk);
    if (slab->aboutToFull()) {
        __slab_list_remove(cache->__full_slabs, slab);
        slab->__next = cache->__used_slabs;
        cache->__used_slabs = slab;
    } else if (slab->empty()) {
        __slab_list_remove(cache->__used_slabs, slab);
        __freePage(slab);
    }
}

}  // namespace nyan::user

static nyan::user::SlabManager __manager;

extern "C" void __fini_libc();

extern "C" {

void exit(int code) {
    __fini_libc();
    sys_exit(code);
}

void _Exit(int code) {
    sys_exit(code);
}

int atoi(const char* str) {
    long result;
    strto(result, str, 0, 10);
    return result;
}

long atol(const char* str) {
    long result;
    strto(result, str, 0, 10);
    return result;
}

long long atoll(const char* str) {
    long long result;
    strto(result, str, 0, 10);
    return result;
}

long strtol(const char* str, char** end, int base) {
    long result;
    if (auto err = strto(result, str, end, base)) {
        errno = err;
    }
    return result;
}

long long strtoll(const char* str, char** end, int base) {
    long long result;
    if (auto err = strto(result, str, end, base)) {
        errno = err;
    }
    return result;
}

unsigned long strtoul(const char* str, char** end, int base) {
    unsigned long result;
    if (auto err = strto(result, str, end, base)) {
        errno = err;
    }
    return result;
}

unsigned long long strtoull(const char* str, char** end, int base) {
    unsigned long long result;
    if (auto err = strto(result, str, end, base)) {
        errno = err;
    }
    return result;
}

void* malloc(size_t size) {
    auto result = __manager.alloc(size);
    if (!result) {
        errno = ENOMEM;
    }
    return result;
}

void free(void* ptr) {
    if (!ptr) {
        return;
    }
    __manager.free(ptr);
}

void* calloc(size_t count, size_t size) {
    auto result = __manager.alloc(count * size);
    if (result) {
        memset(result, 0, count * size);
    } else {
        errno = ENOMEM;
    }
    return result;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return nullptr;
    }
    auto old_size = __manager.allocSize(ptr);
    auto result = __manager.alloc(size);
    if (!result) {
        errno = ENOMEM;
        return nullptr;
    }
    memcpy(result, ptr, min(old_size, size));
    __manager.free(ptr);
    return result;
}
}
