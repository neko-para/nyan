#pragma once

#include <sys/types.h>

#ifdef __BUILDING_NYAN__
#error "kernel shouldn't include this header"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void* lhs, const void* rhs, size_t size);
void* memcpy(void* dst, const void* src, size_t size);
void* memmove(void* dst, const void* src, size_t size);
void* memset(void* dst, int value, size_t size);
void* memchr(const void* src, int ch, size_t size);

char* strcat(char* dst, const char* src);
char* strchr(const char* str, int ch);
int strcmp(const char* lhs, const char* rhs);
char* strcpy(char* dst, const char* src);
size_t strlen(const char* str);
char* strncat(char* dst, const char* src, size_t size);
int strncmp(const char* lhs, const char* rhs, size_t size);
char* strncpy(char* dst, const char* src, size_t size);
char* strrchr(const char* str, int ch);
char* strstr(const char* str, const char* pat);

#ifdef __cplusplus
}
#endif
