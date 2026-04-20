#pragma once

#include <bits/signum.h>
#include <sys/types.h>

#define SA_RESTART 0x10000000
#define SA_NODEFER 0x40000000
#define SA_RESETHAND 0x80000000

__NYAN_BEGIN__

typedef void (*sighandler_t)(int);
typedef uint32_t sigset_t;

struct sigaction {
    sighandler_t sa_handler;
    sigset_t sa_mask;
    int sa_flags;
};

__NYAN_END__
