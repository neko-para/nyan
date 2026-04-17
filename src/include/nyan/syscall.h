#pragma once

#include <nyan/errno.h>
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

// 7
pid_t __NYAN_SYSCALL__(waitpid)(pid_t pid, int* stat_loc, int options);

// 20
pid_t __NYAN_SYSCALL__(getpid)();

// 45
void* __NYAN_SYSCALL__(brk)(const void* addr);

// 162
int __NYAN_SYSCALL__(nanosleep)(const struct timespec* rqtp, struct timespec* rmtp);

__NYAN_SYSCALL_END__
