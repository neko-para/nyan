#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "stdio_impl.h"

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
}
