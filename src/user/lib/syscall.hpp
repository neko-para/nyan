#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

[[noreturn]] void exit(int code);

ssize_t read(int fd, void* buf, size_t size);

ssize_t write(int fd, const void* buf, size_t size);

int getpid();

int nanosleep(const timespec* rqtp, timespec* rmtp);
