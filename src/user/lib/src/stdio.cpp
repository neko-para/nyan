#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "stdio_impl.h"

namespace {

#define __PUT(ch)             \
    do {                      \
        sum++;                \
        if (put((ch), arg)) { \
            return -1;        \
        }                     \
    } while (0)

int printfImpl(bool (*put)(char ch, void* arg), void* arg, const char* format, va_list lst) {
    int sum = 0;
    while (*format) {
        switch (*format) {
            case '%':
                switch (*++format) {
                    case '%':
                        format++;
                        __PUT('%');
                        break;
                    case 'c': {
                        format++;
                        int ch = va_arg(lst, int);
                        __PUT(static_cast<char>(ch));
                        break;
                    }
                    case 's': {
                        format++;
                        const char* str = va_arg(lst, const char*);
                        str = str ? str : "(null)";
                        while (*str) {
                            __PUT(*str++);
                        }
                        break;
                    }
                    case 'd': {
                        format++;
                        int val = va_arg(lst, int);
                        unsigned uval = val;
                        if (val < 0) {
                            __PUT('-');
                            uval = -uval;
                        }
                        char buf[11];
                        char* ptr = buf;
                        do {
                            *ptr++ = (uval % 10) + '0';
                            uval /= 10;
                        } while (uval);
                        while (ptr != buf) {
                            __PUT(*--ptr);
                        }
                        break;
                    }
                    default:
                        return -1;
                }
                break;
            default:
                sum++;
                if (put(*format++, arg)) {
                    return -1;
                }
        }
    }
    return sum;
}

}  // namespace

extern "C" {

FILE* __stdin;
FILE* __stdout;
FILE* __stderr;

FILE __stdin_obj;
FILE __stdout_obj;
FILE __stderr_obj;

void __init_stdio() {
    __stdin = &__stdin_obj;
    __stdout = &__stdout_obj;
    __stderr = &__stderr_obj;
    __stdin->fd = 0;
    __stdout->fd = 1;
    __stderr->fd = 2;
}

int fputc(int ch, FILE* file) {
    uint8_t val = static_cast<uint8_t>(ch);
    auto ret = write(file->fd, &val, 1);
    if (ret < 0) {
        return EOF;
    } else {
        return val;
    }
}

int fputs(const char* str, FILE* file) {
    auto ret = write(file->fd, str, strlen(str));
    if (ret < 0) {
        return EOF;
    } else {
        return 0;
    }
}

int printf(const char* format, ...) {
    va_list lst;
    va_start(lst, format);
    auto ret = vfprintf(stdout, format, lst);
    va_end(lst);
    return ret;
}

int fprintf(FILE* file, const char* format, ...) {
    va_list lst;
    va_start(lst, format);
    auto ret = vfprintf(file, format, lst);
    va_end(lst);
    return ret;
}

int sprintf(char* buf, const char* format, ...) {
    va_list lst;
    va_start(lst, format);
    auto ret = vsprintf(buf, format, lst);
    va_end(lst);
    return ret;
}

int snprintf(char* buf, size_t bufsz, const char* format, ...) {
    va_list lst;
    va_start(lst, format);
    auto ret = vsnprintf(buf, bufsz, format, lst);
    va_end(lst);
    return ret;
}

int vprintf(const char* format, va_list lst) {
    return vfprintf(stdout, format, lst);
}

int vfprintf(FILE* file, const char* format, va_list lst) {
    return printfImpl(
        +[](char ch, void* arg) {
            if (EOF == fputc(ch, static_cast<FILE*>(arg))) {
                return true;
            }
            return false;
        },
        file, format, lst);
}

int vsprintf(char* buf, const char* format, va_list lst) {
    int ret = printfImpl(
        +[](char ch, void* arg) {
            char*& buf = *static_cast<char**>(arg);
            *buf++ = ch;
            return false;
        },
        &buf, format, lst);
    *buf = 0;
    return ret;
}

int vsnprintf(char* buf, size_t bufsz, const char* format, va_list lst) {
    if (bufsz == 0) {
        return printfImpl(+[](char, void*) { return false; }, 0, format, lst);
    } else {
        struct PrintState {
            char* buf;
            char* end;
        } state = {buf, buf + bufsz - 1};
        int ret = printfImpl(
            +[](char ch, void* arg) {
                PrintState* state = static_cast<PrintState*>(arg);
                if (state->buf != state->end) {
                    *state->buf++ = ch;
                }
                return false;
            },
            &state, format, lst);
        *state.buf = 0;
        return ret;
    }
}
}
