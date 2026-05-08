#pragma once

#include <dirent.h>
#include <nyan/errno.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>

namespace nyan::interrupt {
struct SyscallFrame;
}

namespace nyan::syscall {

// 1
[[noreturn]] void exit(int code);

// 2
pid_t fork(interrupt::SyscallFrame* frame);

// 3
ssize_t read(int fd, void* buf, size_t size);

// 4
ssize_t write(int fd, const void* buf, size_t size);

// 5
int open(const char* pathname, int flags, mode_t mode);

// 6
int close(int fd);

// 7
pid_t waitpid(pid_t pid, int* stat_loc, int options);

// 11
int execve(const char* pathname, char* const argv[], char* const envp[], interrupt::SyscallFrame* frame);

// 12
int chdir(const char* pathname);

// 20
pid_t getpid();

// 37
int kill(pid_t pid, int sig);

// 41
int dup(int fd);

// 42
int pipe(int* fds);

// 45
void* brk(const void* addr);

// 48
sighandler_t signal(int sig, sighandler_t handler);

// 54
int ioctl(int fd, uint32_t request, uint32_t param);

// 63
int dup2(int fd, int newFd);

// 90 not impl yet
void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);

// 91
int munmap(void* addr, size_t length);

// 114
pid_t wait4(pid_t pid, int* stat_loc, int options, struct rusage* ru);

// 119
void sigreturn(void* frame);

// 145
ssize_t readv(int fd, const struct iovec* iov, size_t iovcnt);

// 146
ssize_t writev(int fd, const struct iovec* iov, size_t iovcnt);

// 162
int nanosleep(const struct timespec* rqtp, struct timespec* rmtp);

// 174
int rt_sigaction(int sig, const struct sigaction* act, struct sigaction* oldact, size_t sigsetsize);

// 175
int rt_sigprocmask(int how, const sigset_t* set, sigset_t* oldset, size_t sigsetsize);

// 183
char* getcwd(char* buf, size_t size);

// 192
void* mmap2(void* addr, size_t length, int prot, int flags, int fd, uint32_t pgoffset);

// 220
int getdents64(int fd, struct dirent* dirents, unsigned count);

// 221
int fcntl64(int fd, uint32_t request, uint32_t param);

// 224
pid_t gettid();

// 238
int tkill(pid_t tid, int sig);

// 243
int set_thread_area(uint32_t user_desc[4]);

// 252
[[noreturn]] void exit_group(int code);

// 258
int set_tid_address(int* ptr);

// 512
pid_t spawn(const char* name, const char* const* argv, const char* const* envp);

}  // namespace nyan::syscall
