#include <nyan/syscall.h>

#include "utils.hpp"

namespace nyan::syscall {

ssize_t readv(int fd, const iovec* iov, size_t iovcnt) {
    if (!iovcnt || iovcnt > 16) {
        return -SYS_EINVAL;
    }
    if (!utils::validateReadAuto(iov, iovcnt)) {
        return -SYS_EFAULT;
    }

    size_t result = 0;
    for (size_t i = 0; i < iovcnt; i++) {
        if (!iov[i].iov_len) {
            continue;
        }
        auto ret = syscall::read(fd, iov[i].iov_base, iov[i].iov_len);
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
