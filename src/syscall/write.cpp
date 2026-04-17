#include <nyan/syscall.h>

#include "../vga/print.hpp"

namespace nyan::syscall {

ssize_t write(int fd, const void* buf, size_t size) {
    if (fd != 1) {
        return -SYS_EBADF;
    }
    if (!buf) {
        return -SYS_EFAULT;
    }
    if (size > INT_MAX) {
        return -SYS_EINVAL;
    }
    vga::puts(static_cast<const char*>(buf), size);
    return size;
}

}  // namespace nyan::syscall
