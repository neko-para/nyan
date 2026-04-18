#pragma once

#include <sys/types.h>

struct FILE;

#define stdin ((FILE*)0)
#define stdout ((FILE*)1)
#define stderr ((FILE*)2)

#define EOF -1

#ifdef __cplusplus
extern "C" {
#endif

int fputc(int ch, FILE* file);
int fputs(const char* str, FILE* file);

#ifdef __cplusplus
}
#endif
