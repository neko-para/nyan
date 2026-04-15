#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <time.h>

namespace nyan::syscall {

// 1
void exit(int code);

// 3
ssize_t read(int fd, void* buff, size_t size);

// 4
ssize_t write(int fd, const void* buf, size_t size);

// 7
pid_t waitpid(pid_t pid, int* stat_loc, int options);

// 20
pid_t getpid();

// 162
int nanosleep(const timespec* rqtp, timespec* rmtp);

}  // namespace nyan::syscall
