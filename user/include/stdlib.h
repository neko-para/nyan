#pragma once

#include <sys/types.h>

#ifdef __BUILDING_NYAN__
#error "kernel shouldn't include this header"
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#ifdef __cplusplus
extern "C" {
#endif

[[noreturn]] void exit(int code);
[[noreturn]] void _Exit(int code);

int atoi(const char* str);
long atol(const char* str);
long long atoll(const char* str);

long strtol(const char* str, char** end, int base);
long long strtoll(const char* str, char** end, int base);
unsigned long strtoul(const char* str, char** end, int base);
unsigned long long strtoull(const char* str, char** end, int base);

#ifdef __cplusplus
}
#endif
