#include <nyan/syscall.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

ssize_t readv(int fd, const iovec* iov, size_t iovcnt) {
    if (!iovcnt || iovcnt > 16) {
        return SYS_EINVAL;
    }
    __try
        (task::checkR(iov, iovcnt));

    std::vector<iovec> vecs;
    vecs.resize(iovcnt);
    std::copy_n(iov, iovcnt, vecs.data());

    size_t result = 0;
    for (const auto& vec : vecs) {
        if (!vec.iov_len) {
            continue;
        }
        auto ret = syscall::read(fd, vec.iov_base, vec.iov_len);
        if (ret < 0) {
            return result > 0 ? result : ret;
        } else {
            result += ret;
            if (static_cast<size_t>(ret) < vec.iov_len) {
                return result;
            }
        }
    }
    return result;
}

}  // namespace nyan::syscall
