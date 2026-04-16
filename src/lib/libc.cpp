#include <sys/types.h>

#include "../vga/print.hpp"

struct __llvm_libc_stdio_cookie {
    int32_t fd;
};

__llvm_libc_stdio_cookie __llvm_libc_stdin_cookie = {0};
__llvm_libc_stdio_cookie __llvm_libc_stdout_cookie = {1};
__llvm_libc_stdio_cookie __llvm_libc_stderr_cookie = {2};

extern "C" ssize_t __llvm_libc_stdio_read([[maybe_unused]] __llvm_libc_stdio_cookie* cookie,
                                          [[maybe_unused]] char* buf,
                                          [[maybe_unused]] size_t size) {
    return 0;
}

extern "C" ssize_t __llvm_libc_stdio_write(__llvm_libc_stdio_cookie* cookie, const char* buf, size_t size) {
    if (cookie->fd != 1 && cookie->fd != 2) {
        return 0;
    }
    nyan::vga::puts(buf, size);
    return size;
}

static int __errno;

extern "C" int* __llvm_libc_errno() {
    return &__errno;
}
