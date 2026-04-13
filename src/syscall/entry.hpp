#pragma once

#include <time.h>

namespace nyan::syscall {

// 1
void exit(int code);

// 20
int getpid();

// 162
int nanosleep(const timespec* rqtp, timespec* rmtp);

}  // namespace nyan::syscall
