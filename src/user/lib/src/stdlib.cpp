#include <stdlib.h>

extern "C" {

int atoi(const char* str) {
    int value = 0;
    bool sign = false;
    if (*str == '-') {
        str++;
        sign = true;
    } else if (*str == '+') {
        str++;
    }
    while (*str >= '0' && *str <= '9') {
        value *= 10;
        value += *str++ - '0';
    }
    return sign ? -value : value;
}
}
