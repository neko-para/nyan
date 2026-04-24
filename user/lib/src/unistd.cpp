#include <errno.h>
#include <nyan/syscall.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "utils.hpp"

extern "C" {

pid_t spawn(const char* name, const char* const* argv) {
    return sys_spawn(name, argv);
}

pid_t fork() {
    return wrapRet(sys_fork());
}

int execve(const char* pathname, char* const* argv, char* const* envp) {
    return wrapRet(sys_execve(pathname, argv, envp));
}

ssize_t read(int fd, void* buf, size_t size) {
    return wrapRet(sys_read(fd, buf, size));
}

ssize_t write(int fd, const void* buf, size_t size) {
    return wrapRet(sys_write(fd, buf, size));
}

int close(int fd) {
    return wrapRet(sys_close(fd));
}

int dup(int fd) {
    return wrapRet(sys_dup(fd));
}

int dup2(int fd, int new_fd) {
    return wrapRet(sys_dup2(fd, new_fd));
}

int pipe(int* fds) {
    return wrapRet(sys_pipe(fds));
}

pid_t waitpid(pid_t pid, int* stat_loc, int options) {
    return wrapRet(sys_waitpid(pid, stat_loc, options));
}

pid_t getpid() {
    return sys_getpid();
}

int brk(const void* addr) {
    auto new_addr = sys_brk(addr);
    if (new_addr < addr) {
        errno = ENOMEM;
        return -1;
    }
    return 0;
}

void* sbrk(intptr_t increment) {
    auto old_addr = sys_brk(0);
    if (increment == 0) {
        return old_addr;
    }
    auto new_addr = sys_brk(reinterpret_cast<void*>(reinterpret_cast<intptr_t>(old_addr) + increment));
    if (old_addr == new_addr) {
        errno = ENOMEM;
        return reinterpret_cast<void*>(-1);
    } else {
        return old_addr;
    }
}

int nanosleep(const timespec* rqtp, timespec* rmtp) {
    return wrapRet(sys_nanosleep(rqtp, rmtp));
}

unsigned sleep(unsigned sec) {
    timespec rq = {
        .tv_sec = static_cast<time_t>(sec),
        .tv_nsec = 0,
    };
    timespec rm = {};
    if (-1 == nanosleep(&rq, &rm)) {
        return sec;
    } else {
        return static_cast<unsigned>(rm.tv_sec);
    }
}

int tcsetpgrp(int fd, pid_t pgid) {
    return ioctl(fd, TIOCSPGRP, pgid);
}
}
