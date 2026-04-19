#pragma once

#include <sys/signal.h>
#include <sys/types.h>

#ifdef __BUILDING_NYAN__
#error "kernel shouldn't include this header"
#endif

#ifdef __cplusplus
extern "C" {
#endif

__sighandler_t signal(int sig, __sighandler_t handler);
int kill(pid_t pid, int sig);
int raise(int sig);

#ifdef __cplusplus
}
#endif
