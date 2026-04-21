#pragma once

#include <sys/types.h>

#ifdef __BUILDING_NYAN__
#error "kernel shouldn't include this header"
#endif

#ifdef __cplusplus
extern "C" {
#endif

pid_t spawn(const char* name, const char* const* argv);

ssize_t read(int fd, void* buf, size_t size);

ssize_t write(int fd, const void* buf, size_t size);

int close(int fd);

int dup(int fd);
int dup2(int fd, int new_fd);

pid_t waitpid(pid_t pid, int* stat_loc, int options);

pid_t getpid();

int brk(const void* addr);
void* sbrk(intptr_t increment);

int nanosleep(const struct timespec* rqtp, struct timespec* rmtp);

int tcsetpgrp(int fd, pid_t pgid);

#ifdef __cplusplus
}
#endif
