#pragma once

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

#ifdef __cplusplus
}
#endif
