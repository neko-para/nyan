#pragma once

#include <stddef.h>
#include <stdint.h>

#include <nyan/types.h>

__NYAN_BEGIN__

typedef int32_t pid_t;
typedef int32_t ssize_t;

typedef int32_t time_t;

struct timespec {
    time_t tv_sec;
    int32_t tv_nsec;
};

__NYAN_END__
