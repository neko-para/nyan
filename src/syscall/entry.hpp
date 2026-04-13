#pragma once

#include <stdio.h>
#include <time.h>

namespace nyan::syscall {

// 1
void exit(int code);

// 4
ssize_t write(int fd, const void* buf, size_t size);

// 20
int getpid();

// 162
int nanosleep(const timespec* rqtp, timespec* rmtp);

}  // namespace nyan::syscall
