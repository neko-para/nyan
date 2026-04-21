#include <ctype.h>
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
    while (isdigit(*str)) {
        value *= 10;
        value += *str++ - '0';
    }
    return sign ? -value : value;
}
}
