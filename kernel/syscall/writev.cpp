#include <nyan/syscall.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

ssize_t writev(int fd, const struct iovec* vec, size_t vlen) {
    if (!vlen || vlen > 16) {
        return SYS_EINVAL;
    }
    __try
        (task::checkR(vec, vlen));

    std::vector<iovec> vecs;
    vecs.resize(vlen);
    std::copy_n(vec, vlen, vecs.data());

    size_t result = 0;
    for (const auto& v : vecs) {
        if (!v.iov_len) {
            continue;
        }
        auto ret = syscall::write(fd, v.iov_base, v.iov_len);
        if (ret < 0) {
            return result > 0 ? result : ret;
        } else {
            result += ret;
            if (static_cast<size_t>(ret) < v.iov_len) {
                return result;
            }
        }
    }
    return result;
}

}  // namespace nyan::syscall
