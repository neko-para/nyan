#pragma once

#include <sys/types.h>

#define WEXITSTATUS(s) (((s) >> 8) & 0xFF)
#define WTERMSIG(s) ((s) & 0x7F)
#define WIFEXITED(s) (WTERMSIG(s) == 0)
#define WIFSIGNALED(s) (((signed char)(((s) & 0x7F) + 1) >> 1) > 0)
#define WIFSTOPPED(s) (((s) & 0xFF) == 0x7F)
#define WSTOPSIG(s) WEXITSTATUS(s)

// waitpid options
#define WNOHANG 1
// #define WUNTRACED 2
