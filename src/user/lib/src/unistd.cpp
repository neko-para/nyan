#include <errno.h>
#include <nyan/syscall.h>
#include <unistd.h>

template <typename T>
static T wrapRet(T ret) {
    if (ret < 0) {
        errno = -ret;
        return -1;
    } else {
        return ret;
    }
}

void exit(int code) {
    sys_exit(code);
}

pid_t spawn(const char* name) {
    return sys_spawn(name);
}

ssize_t read(int fd, void* buf, size_t size) {
    return wrapRet(sys_read(fd, buf, size));
}

ssize_t write(int fd, const void* buf, size_t size) {
    return wrapRet(sys_write(fd, buf, size));
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

int nanosleep(const struct timespec* rqtp, struct timespec* rmtp) {
    return wrapRet(sys_nanosleep(rqtp, rmtp));
}
