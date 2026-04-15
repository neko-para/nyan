#include <unistd.h>

extern "C" [[noreturn]] void exit(int code) {
    asm volatile("int $0x80" ::"a"(1), "b"(code));
    __builtin_unreachable();
}

extern "C" ssize_t read(int fd, void* buf, size_t size) {
    ssize_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(3), "b"(fd), "c"(buf), "d"(size));
    return ret;
}

extern "C" ssize_t write(int fd, const void* buf, size_t size) {
    ssize_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(4), "b"(fd), "c"(buf), "d"(size));
    return ret;
}

extern "C" pid_t waitpid(pid_t pid, int* stat_loc, int options) {
    pid_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(4), "b"(pid), "c"(stat_loc), "d"(options));
    return ret;
}

extern "C" pid_t getpid() {
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(20));
    return ret;
}

extern "C" int nanosleep(const timespec* rqtp, timespec* rmtp) {
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(20), "b"(rqtp), "c"(rmtp));
    return ret;
}
