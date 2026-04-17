#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern "C" {

int fputs(const char* str, FILE* file) {
    auto ret = write(reinterpret_cast<int>(file), str, strlen(str));
    if (ret < 0) {
        return EOF;
    } else {
        return 0;
    }
}
}
