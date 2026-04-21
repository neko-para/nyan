#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <nyan/syscall.h>
#include <stdlib.h>
#include <string.h>

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
}
