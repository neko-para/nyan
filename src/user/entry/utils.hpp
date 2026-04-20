#pragma once

#include <stdio.h>

inline void printNum(int val, FILE* file) {
    if (val < 0) {
        fputc('-', file);
        val = -val;
    }
    char buf[11];
    char* ptr = buf;
    do {
        *ptr++ = (val % 10) + '0';
        val /= 10;
    } while (val);
    while (ptr != buf) {
        fputc(*--ptr, file);
    }
}
