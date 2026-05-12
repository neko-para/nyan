#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

ssize_t read(int fd, void* buf, size_t count) {
    if (count > INT_MAX) {
        return SYS_EINVAL;
    }
    __try
        (task::checkW(buf, count));

    return __try(task::getFd(fd))->read(buf, count).merge();
}

}  // namespace nyan::syscall
