#pragma once

#include <bits/signum.h>
#include <sys/types.h>

__NYAN_BEGIN__

typedef void (*sighandler_t)(int);
typedef uint32_t sigset_t;

struct sigaction {
    sighandler_t sa_handler;
    sigset_t sa_mask;
    int sa_flags;
};

__NYAN_END__
