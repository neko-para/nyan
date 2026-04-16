#pragma once

#include <sys/types.h>

#ifdef __BUILDING_NYAN__
#error "kernel shouldn't include this header"
#endif

#ifdef __cplusplus
extern "C" {
#endif

[[noreturn]] void exit(int code);

pid_t spawn(const char* name);

ssize_t read(int fd, void* buf, size_t size);

ssize_t write(int fd, const void* buf, size_t size);

pid_t waitpid(pid_t pid, int* stat_loc, int options);

pid_t getpid();

int nanosleep(const struct timespec* rqtp, struct timespec* rmtp);

#ifdef __cplusplus
}
#endif
