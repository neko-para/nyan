#include <nyan/syscall.h>

#include "../console/entry.hpp"
#include "../task/guard.hpp"

namespace nyan::syscall {

ssize_t read(int fd, void* buf, size_t size) {
    if (!buf) {
        return -SYS_EFAULT;
    }
    if (size > INT_MAX) {
        return -SYS_EINVAL;
    }
    if (fd == 0) {
        if (auto tty = task::currentTask->tty) {
            auto guard = tty->syncWaitInput();
            auto result = std::min(tty->inputBuffer.size(), size);
            std::copy_n(tty->inputBuffer.data(), result, static_cast<uint8_t*>(buf));
            tty->inputBuffer.erase(0, result);
            return result;
        } else {
            return -SYS_EBADF;
        }
    } else {
        return -SYS_EBADF;
    }
}

}  // namespace nyan::syscall
