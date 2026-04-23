#include <string.h>

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

char* strcat(char* dst, const char* src) {
    auto pdst = dst;
    while (*pdst) {
        pdst++;
    }
    while ((*pdst++ = *src++)) {
    }
    return dst;
}

char* strchr(const char* str, int ch) {
    if (ch == 0) {
        while (*str) {
            str++;
        }
        return const_cast<char*>(str);
    } else {
        while (*str) {
            if (*str == ch) {
                return const_cast<char*>(str);
            } else {
                str++;
            }
        }
        return nullptr;
    }
}

int strcmp(const char* lhs, const char* rhs) {
    while (*lhs == *rhs && *lhs) {
        lhs++;
        rhs++;
    }
    return *lhs - *rhs;
}

char* strcpy(char* dst, const char* src) {
    auto pdst = dst;
    while ((*pdst++ = *src++)) {
    }
    return dst;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

char* strncat(char* dst, const char* src, size_t size) {
    auto pdst = dst;
    while (*pdst) {
        pdst++;
    }
    while (size--) {
        if (!(*pdst++ = *src++)) {
            return dst;
        }
    }
    *pdst = 0;
    return dst;
}

int strncmp(const char* lhs, const char* rhs, size_t size) {
    while (size--) {
        if (*lhs != *rhs || !*lhs) {
            return *lhs - *rhs;
        }
        lhs++;
        rhs++;
    }
    return 0;
}

char* strncpy(char* dst, const char* src, size_t size) {
    auto pdst = dst;
    while (size && (*pdst++ = *src++)) {
        size--;
    }
    while (size--) {
        *pdst++ = 0;
    }
    return dst;
}

char* strrchr(const char* str, int ch) {
    if (ch == 0) {
        while (*str) {
            str++;
        }
        return const_cast<char*>(str);
    } else {
        const char* result = nullptr;
        while (*str) {
            if (*str == ch) {
                result = str++;
            } else {
                str++;
            }
        }
        return const_cast<char*>(result);
    }
}

char* strstr(const char* str, const char* pat) {
    size_t len = strlen(pat);
    if (!len) {
        return const_cast<char*>(str);
    }
    while (true) {
        for (size_t i = 0; i < len; i++) {
            if (!str[i]) {
                return nullptr;
            }
            if (str[i] != pat[i]) {
                goto fail;
            }
        }
        return const_cast<char*>(str);
    fail:
        str++;
    }
}
}
