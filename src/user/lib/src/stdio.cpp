#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern "C" {

int fputc(int ch, FILE* file) {
    uint8_t val = static_cast<uint8_t>(ch);
    auto ret = write(reinterpret_cast<int>(file), &val, 1);
    if (ret < 0) {
        return EOF;
    } else {
        return val;
    }
}

int fputs(const char* str, FILE* file) {
    auto ret = write(reinterpret_cast<int>(file), str, strlen(str));
    if (ret < 0) {
        return EOF;
    } else {
        return 0;
    }
}
}
