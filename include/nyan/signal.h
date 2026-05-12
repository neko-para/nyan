#pragma once

#include <sys/types.h>

using __nyan_sigset = uint64_t;

struct __nyan_sigaction {
    void (*__handler)(int);
    unsigned long __flags;
    void (*__restorer)(void);
    __nyan_sigset __mask;
};
