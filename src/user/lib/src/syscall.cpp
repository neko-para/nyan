#include <nyan/syscall.h>

template <size_t Id, typename Ret = void>
static Ret syscall() {
    if constexpr (__is_same(Ret, void)) {
        asm volatile("int $0x80" ::"a"(Id));
    } else {
        Ret ret;
        asm volatile("int $0x80" : "=a"(ret) : "a"(Id));
        return ret;
    }
}

template <size_t Id, typename Ret, typename Arg1>
static Ret syscall(Arg1 arg1) {
    if constexpr (__is_same(Ret, void)) {
        asm volatile("int $0x80" ::"a"(Id), "b"(arg1));
    } else {
        Ret ret;
        asm volatile("int $0x80" : "=a"(ret) : "a"(Id), "b"(arg1));
        return ret;
    }
}

template <size_t Id, typename Ret, typename Arg1, typename Arg2>
static Ret syscall(Arg1 arg1, Arg2 arg2) {
    if constexpr (__is_same(Ret, void)) {
        asm volatile("int $0x80" ::"a"(Id), "b"(arg1), "c"(arg2));
    } else {
        Ret ret;
        asm volatile("int $0x80" : "=a"(ret) : "a"(Id), "b"(arg1), "c"(arg2));
        return ret;
    }
}

template <size_t Id, typename Ret, typename Arg1, typename Arg2, typename Arg3>
static Ret syscall(Arg1 arg1, Arg2 arg2, Arg3 arg3) {
    if constexpr (__is_same(Ret, void)) {
        asm volatile("int $0x80" ::"a"(Id), "b"(arg1), "c"(arg2), "d"(arg3));
    } else {
        Ret ret;
        asm volatile("int $0x80" : "=a"(ret) : "a"(Id), "b"(arg1), "c"(arg2), "d"(arg3));
        return ret;
    }
}

extern "C" {

[[noreturn]] void sys_exit(int code) {
    syscall<1, void>(code);
    __builtin_unreachable();
}

pid_t sys_spawn(const char* name, const char* const* argv) {
    return syscall<2, pid_t>(name, argv);
}

ssize_t sys_read(int fd, void* buf, size_t size) {
    return syscall<3, ssize_t>(fd, buf, size);
}

ssize_t sys_write(int fd, const void* buf, size_t size) {
    return syscall<4, ssize_t>(fd, buf, size);
}

pid_t sys_waitpid(pid_t pid, int* stat_loc, int options) {
    return syscall<7, pid_t>(pid, stat_loc, options);
}

pid_t sys_getpid() {
    return syscall<20, pid_t>();
}

int sys_kill(pid_t pid, int sig) {
    return syscall<37, int>(pid, sig);
}

void* sys_brk(const void* addr) {
    return syscall<45, void*>(addr);
}

sighandler_t sys_signal(int sig, sighandler_t handler) {
    return syscall<48, sighandler_t>(sig, handler);
}

int sys_nanosleep(const timespec* rqtp, timespec* rmtp) {
    return syscall<162, int>(rqtp, rmtp);
}
}
