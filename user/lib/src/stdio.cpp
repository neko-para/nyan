#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "stdio_impl.h"
#include "utils.hpp"

using namespace nyan::user;

namespace {

#define __PUT(ch)             \
    do {                      \
        sum++;                \
        if (put((ch), arg)) { \
            return -1;        \
        }                     \
    } while (0)

#define __FLUSH(file)              \
    do {                           \
        if (EOF == fflush(file)) { \
            return EOF;            \
        }                          \
    } while (false)

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

__attribute__((section(".bss"))) char stdin_buffer[BUFSIZ];
__attribute__((section(".bss"))) char stdout_buffer[BUFSIZ];

void __init_stdio() {
    __stdin = &__stdin_obj;
    __stdout = &__stdout_obj;
    __stderr = &__stderr_obj;

    stdin->fd = 0;
    stdin->flags = F_In | F_FullBuf;
    stdin->buf = stdin_buffer;
    stdin->buf_size = BUFSIZ;
    stdin->buf_pos = 0;
    stdin->buf_end = 0;

    stdout->fd = 1;
    stdout->flags = F_Out | F_LineBuf;
    stdout->buf = stdout_buffer;
    stdout->buf_size = BUFSIZ;
    stdout->buf_pos = 0;
    stdout->buf_end = 0;

    stderr->fd = 2;
    stderr->flags = F_Out | F_NoBuf;
    stderr->buf = 0;
    stderr->buf_size = 0;
    stderr->buf_pos = 0;
    stderr->buf_end = 0;
}

void __fini_stdio() {
    fflush(stdout);
}

int fputc(int ch, FILE* file) {
    if (!(file->flags & F_Out)) {
        errno = EBADF;
        file->flags |= F_Err;
        return EOF;
    }

    uint8_t val = static_cast<uint8_t>(ch);
    if (file->bufMode() == F_NoBuf) {
        auto ret = write(file->fd, &val, 1);
        if (ret <= 0) {
            file->flags |= F_Err;
            return EOF;
        } else {
            return val;
        }
    }

    if (file->isBufFull()) {
        __FLUSH(file);
    }

    file->buf[file->buf_pos++] = val;
    if (file->bufMode() == F_LineBuf && val == '\n') {
        __FLUSH(file);
    }
    return val;
}

int fputs(const char* str, FILE* file) {
    if (!(file->flags & F_Out)) {
        errno = EBADF;
        file->flags |= F_Err;
        return EOF;
    }

    if (file->bufMode() == F_NoBuf) {
        size_t len = strlen(str);
        while (len > 0) {
            auto ret = write(file->fd, str, len);
            if (ret <= 0) {
                file->flags |= F_Err;
                return EOF;
            } else {
                len -= ret;
                str += ret;
            }
        }
        return 0;
    }

    if (file->bufMode() == F_LineBuf) {
        if (auto ptr = strrchr(str, '\n')) {
            __FLUSH(file);
            ptr++;

            size_t len = ptr - str;
            while (len > 0) {
                auto ret = write(file->fd, str, len);
                if (ret <= 0) {
                    file->flags |= F_Err;
                    return EOF;
                } else {
                    len -= ret;
                    str += ret;
                }
            }
        }
    }

    size_t len = strlen(str);
    while (len > 0) {
        size_t rest = file->buf_size - file->buf_pos;
        size_t size = min(len, rest);
        memcpy(file->buf + file->buf_pos, str, size);
        file->buf_pos += size;
        str += size;
        len -= size;
        if (file->isBufFull()) {
            __FLUSH(file);
        }
    }

    return 0;
}

char* fgets(char* buf, int size, FILE* file) {
    if (!file) {
        errno = EINVAL;
        return 0;
    }
    if (!(file->flags & F_In)) {
        errno = EBADF;
        file->flags |= F_Err;
        return 0;
    }
    if (size <= 0) {
        errno = EINVAL;
        return 0;
    }

    if (file->bufMode() == F_NoBuf) {
        // TODO: impl
        file->flags |= F_Err;
        return 0;
    }

    auto cur = buf;
    size_t restSize = size - 1;
    while (true) {
        if (file->buf_pos == file->buf_end) {
            auto ret = read(file->fd, file->buf, file->buf_size);
            if (ret < 0) {
                file->flags |= F_Err;
                if (cur != buf) {
                    *cur = 0;
                    return buf;
                }
                return 0;
            } else if (ret == 0) {
                file->buf_pos = 0;
                file->buf_end = 0;
                file->flags |= F_Eof;
                if (cur != buf) {
                    *cur = 0;
                    return buf;
                }
                return 0;
            } else {
                file->buf_pos = 0;
                file->buf_end = ret;
            }
        }

        auto start = file->buf + file->buf_pos;
        auto ptr = static_cast<char*>(memchr(start, '\n', file->buf_end - file->buf_pos));
        auto fill = ptr ? (ptr + 1) : (file->buf + file->buf_end);
        size_t rest = fill - start;
        size_t len = min(rest, restSize);
        memcpy(cur, start, len);
        cur += len;
        restSize -= len;
        file->buf_pos += len;
        if (ptr || restSize == 0) {
            *cur = 0;
            return buf;
        }
    }
}

int fflush(FILE* file) {
    if (!file) {
        __FLUSH(stdout);
        __FLUSH(stderr);
        return 0;
    }
    if (!(file->flags & F_Out)) {
        return 0;
    }
    if (file->buf && file->buf_pos > 0) {
        while (file->buf_pos > 0) {
            auto ret = write(file->fd, file->buf, file->buf_pos);
            if (ret <= 0) {
                file->flags |= F_Err;
                return EOF;
            } else {
                memmove(file->buf, file->buf + ret, file->buf_pos - ret);
                file->buf_pos -= ret;
            }
        }
        return 0;
    } else {
        return 0;
    }
}

void clearerr(FILE* file) {
    if (file) {
        file->flags &= ~(F_Err | F_Eof);
    }
}

int feof(FILE* file) {
    if (file) {
        return !!(file->flags & F_Eof);
    } else {
        return 0;
    }
}

int ferror(FILE* file) {
    if (file) {
        return !!(file->flags & F_Err);
    } else {
        return 0;
    }
}

int fileno(FILE* file) {
    if (file) {
        return file->fd;
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
            // TODO: 直接操作buf
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
