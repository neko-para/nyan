#include "entry.hpp"

#include <errno.h>

#include "../keyboard/buffer.hpp"
#include "../task/guard.hpp"

namespace nyan::syscall {

static bool empty() {
    task::InterruptGuard guard;
    return keyboard::buffer.empty();
}

ssize_t read(int fd, void* buf, size_t size) {
    if (fd != 0) {
        return -EBADF;
    }
    if (!buf) {
        return -EFAULT;
    }
    if (size > INT_MAX) {
        return -EINVAL;
    }
    while (empty()) {
        keyboard::waitList.wait();
    }
    task::InterruptGuard guard;
    return keyboard::buffer.popSome(static_cast<uint8_t*>(buf), size);
}

}  // namespace nyan::syscall
