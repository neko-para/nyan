#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../task/mod.hpp"
#include "utils.hpp"

namespace nyan::syscall {

ssize_t read(int fd, void* buf, size_t size) {
    if (size > INT_MAX) {
        return SYS_EINVAL;
    }
    if (!utils::validateWrite(buf, size)) {
        return SYS_EFAULT;
    }

    return __try(task::getFd(fd))->read(buf, size).merge();
}

}  // namespace nyan::syscall
