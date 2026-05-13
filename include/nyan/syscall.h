#pragma once

#include <dirent.h>
#include <fcntl.h>
#include <nyan/errno.h>
#include <nyan/signal.h>
#include <nyan/time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

namespace nyan::interrupt {
struct SyscallFrame;
}

namespace nyan::syscall {

// 1
[[noreturn]] void exit(int error_code);

// 2
pid_t fork(interrupt::SyscallFrame* frame);

// 3
ssize_t read(int fd, void* buf, size_t count);

// 4
ssize_t write(int fd, const void* buf, size_t count);

// 5
int open(const char* filename, int flags, mode_t mode);

// 6
int close(int fd);

// 7
pid_t waitpid(pid_t pid, int* stat_addr, int options);

// 11
int execve(const char* filename, const char* const* argv, const char* const* envp, interrupt::SyscallFrame* frame);

// 12
int chdir(const char* filename);

// 20
pid_t getpid();

// 37
int kill(pid_t pid, int sig);

// 41
int dup(int fildes);

// 42
int pipe(int* fildes);

// 45
void* brk(void* brk);

// 54
int ioctl(int fd, unsigned cmd, uint32_t arg);

// 57
int setpgid(pid_t pid, pid_t pgid);

// 63
int dup2(int oldfd, int newfd);

// 64
pid_t getppid();

// 65
pid_t getpgrp();

// 91
int munmap(void* addr, size_t len);

// 114
pid_t wait4(pid_t upid, int* stat_addr, int options, struct rusage* ru);

// 119
void sigreturn(interrupt::SyscallFrame* frame);

// 132
pid_t getpgid(pid_t pid);

// 140
int llseek(int fd, uint32_t offset_high, uint32_t offset_low, loff_t* result, int whence);

// 145
ssize_t readv(int fd, const struct iovec* vec, size_t vlen);

// 146
ssize_t writev(int fd, const struct iovec* vec, size_t vlen);

// 162
int nanosleep(const __nyan_timespec32* rqtp, __nyan_timespec32* rmtp);

// 174
int rt_sigaction(int sig, const __nyan_sigaction* act, __nyan_sigaction* oact, size_t sigsetsize);

// 175
int rt_sigprocmask(int how, const __nyan_sigset* nset, __nyan_sigset* oset, size_t sigsetsize);

// 183
char* getcwd(char* buf, size_t size);

// 192
void* mmap_pgoff(void* addr, size_t len, int prot, int flags, int fd, size_t pgoff);

// 195
int stat64(const char* filename, struct stat* statbuf);

// 220
int getdents64(int fd, struct dirent* dirent, size_t count);

// 221
int fcntl64(int fd, unsigned cmd, uint32_t arg);

// 224
pid_t gettid();

// 238
int tkill(pid_t pid, int sig);

// 243
int set_thread_area(uint32_t* u_info);

// 252
[[noreturn]] void exit_group(int error_code);

// 258
int set_tid_address(int* tidptr);

// 512
pid_t spawn(const char* name, const char* const* argv, const char* const* envp);

}  // namespace nyan::syscall
