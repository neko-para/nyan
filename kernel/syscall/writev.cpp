#include <nyan/syscall.h>

namespace nyan::syscall {

ssize_t writev(int fd, const struct iovec* iov, size_t iovcnt) {
    if (!iov) {
        return -SYS_EFAULT;
    }
    size_t result = 0;
    for (size_t i = 0; i < iovcnt; i++) {
        if (!iov[i].iov_len) {
            continue;
        }
        auto ret = syscall::write(fd, iov[i].iov_base, iov[i].iov_len);
        if (ret < 0) {
            return result > 0 ? result : ret;
        } else {
            result += ret;
            if (static_cast<size_t>(ret) < iov[i].iov_len) {
                return result;
            }
        }
    }
    return result;
}

}  // namespace nyan::syscall
