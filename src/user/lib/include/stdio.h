#pragma once

#include <stdarg.h>
#include <sys/types.h>

#ifdef __BUILDING_NYAN__
#error "kernel shouldn't include this header"
#endif

struct FILE;

#ifdef __cplusplus
extern "C" FILE* __stdin;
extern "C" FILE* __stdout;
extern "C" FILE* __stderr;
#else
extern FILE* __stdin;
extern FILE* __stdout;
extern FILE* __stderr;
#endif

#define stdin (__stdin)
#define stdout (__stdout)
#define stderr (__stderr)

#define EOF -1
#define BUFSIZ 8192

#ifdef __cplusplus
extern "C" {
#endif

int fputc(int ch, FILE* file);
int fputs(const char* str, FILE* file);

int printf(const char* format, ...) __attribute__((format(printf, 1, 2)));
int fprintf(FILE* file, const char* format, ...) __attribute__((format(printf, 2, 3)));
int sprintf(char* buf, const char* format, ...) __attribute__((format(printf, 2, 3)));
int snprintf(char* buf, size_t bufsz, const char* format, ...) __attribute__((format(printf, 3, 4)));

int vprintf(const char* format, va_list lst);
int vfprintf(FILE* file, const char* format, va_list lst);
int vsprintf(char* buf, const char* format, va_list lst);
int vsnprintf(char* buf, size_t bufsz, const char* format, va_list lst);

#ifdef __cplusplus
}
#endif
