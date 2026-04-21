#pragma once

#include <nyan/errno.h>
#include <sys/signal.h>
#include <sys/types.h>

__NYAN_SYSCALL_BEGIN__

// 1
[[noreturn]] void __NYAN_SYSCALL__(exit)(int code);

// 2, 借用fork
pid_t __NYAN_SYSCALL__(spawn)(const char* name, const char* const* argv);

// 3
ssize_t __NYAN_SYSCALL__(read)(int fd, void* buf, size_t size);

// 4
ssize_t __NYAN_SYSCALL__(write)(int fd, const void* buf, size_t size);

// 6
int __NYAN_SYSCALL__(close)(int fd);

// 7
pid_t __NYAN_SYSCALL__(waitpid)(pid_t pid, int* stat_loc, int options);

// 20
pid_t __NYAN_SYSCALL__(getpid)();

// 37
int __NYAN_SYSCALL__(kill)(pid_t pid, int sig);

// 45
void* __NYAN_SYSCALL__(brk)(const void* addr);

// 48
sighandler_t __NYAN_SYSCALL__(signal)(int sig, sighandler_t handler);

// 54
int __NYAN_SYSCALL__(ioctl)(int fd, uint32_t request, uint32_t param);

// 119
void __NYAN_SYSCALL__(sigreturn)(void* frame);

// 162
int __NYAN_SYSCALL__(nanosleep)(const struct timespec* rqtp, struct timespec* rmtp);

__NYAN_SYSCALL_END__
