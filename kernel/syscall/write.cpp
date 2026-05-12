#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

ssize_t write(int fd, const char* buf, size_t size) {
    if (size > INT_MAX) {
        return SYS_EINVAL;
    }
    __try
        (task::checkR(buf, size));

    return __try(task::getFd(fd))->write(buf, size).merge();
}

}  // namespace nyan::syscall
