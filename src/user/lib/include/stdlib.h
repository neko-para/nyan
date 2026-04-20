#pragma once

#include <sys/types.h>

#ifdef __BUILDING_NYAN__
#error "kernel shouldn't include this header"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int atoi(const char* str);

#ifdef __cplusplus
}
#endif
