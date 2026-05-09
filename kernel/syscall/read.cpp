#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

ssize_t read(int fd, void* buf, size_t size) {
    if (size > INT_MAX) {
        return SYS_EINVAL;
    }
    __try
        (task::checkW(buf, size));

    return __try(task::getFd(fd))->read(buf, size).merge();
}

}  // namespace nyan::syscall
