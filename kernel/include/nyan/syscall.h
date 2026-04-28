#pragma once

#include <nyan/errno.h>
#include <nyan/types.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>

__NYAN_SYSCALL_BEGIN__

// 1
[[noreturn]] void __NYAN_SYSCALL__(exit)(int code);

// 2
pid_t __NYAN_SYSCALL__(fork)(__NYAN_SYSCALL_FRAME__);

// 3
ssize_t __NYAN_SYSCALL__(read)(int fd, void* buf, size_t size);

// 4
ssize_t __NYAN_SYSCALL__(write)(int fd, const void* buf, size_t size);

// 6
int __NYAN_SYSCALL__(close)(int fd);

// 7
pid_t __NYAN_SYSCALL__(waitpid)(pid_t pid, int* stat_loc, int options);

// 11
int __NYAN_SYSCALL__(execve)(const char* pathname, char* const argv[], char* const envp[] __NYAN_SYSCALL_FRAME2__);

// 20
pid_t __NYAN_SYSCALL__(getpid)();

// 37
int __NYAN_SYSCALL__(kill)(pid_t pid, int sig);

// 41
int __NYAN_SYSCALL__(dup)(int fd);

// 42
int __NYAN_SYSCALL__(pipe)(int* fds);

// 45
void* __NYAN_SYSCALL__(brk)(const void* addr);

// 48
sighandler_t __NYAN_SYSCALL__(signal)(int sig, sighandler_t handler);

// 54
int __NYAN_SYSCALL__(ioctl)(int fd, uint32_t request, uint32_t param);

// 63
int __NYAN_SYSCALL__(dup2)(int fd, int newFd);

// 90
void* __NYAN_SYSCALL__(mmap)(void* addr, size_t length, int prot, int flags, int fd, off_t offset);

// 91
void __NYAN_SYSCALL__(munmap)(void* addr, size_t length);

// 114
pid_t __NYAN_SYSCALL__(wait4)(pid_t pid, int* stat_loc, int options, struct rusage* ru);

// 119
void __NYAN_SYSCALL__(sigreturn)(void* frame);

// 145
ssize_t __NYAN_SYSCALL__(readv)(int fd, const struct iovec* iov, size_t iovcnt);

// 146
ssize_t __NYAN_SYSCALL__(writev)(int fd, const struct iovec* iov, size_t iovcnt);

// 162
int __NYAN_SYSCALL__(nanosleep)(const struct timespec* rqtp, struct timespec* rmtp);

// 175
int __NYAN_SYSCALL__(rt_sigprocmask)(int how, const sigset_t* set, sigset_t* oldset, size_t sigsetsize);

// 243
int __NYAN_SYSCALL__(set_thread_area)(uint32_t user_desc[4]);

// 252
[[noreturn]] void __NYAN_SYSCALL__(exit_group)(int code);

// 258
int __NYAN_SYSCALL__(set_tid_address)(int* ptr);

// 512
pid_t __NYAN_SYSCALL__(spawn)(const char* name, const char* const* argv);

__NYAN_SYSCALL_END__
