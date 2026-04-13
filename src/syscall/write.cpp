#include "entry.hpp"

#include <errno.h>

#include "../vga/print.hpp"

namespace nyan::syscall {

ssize_t write(int fd, const void* buf, size_t size) {
    if (fd != 1) {
        return -EBADF;
    }
    if (!buf) {
        return -EFAULT;
    }
    if (size > INT_MAX) {
        return -EINVAL;
    }
    vga::puts(static_cast<const char*>(buf), size);
    return size;
}

}  // namespace nyan::syscall
