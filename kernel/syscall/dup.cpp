#include <nyan/syscall.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

int dup(int fd) {
    auto fdObj = __try(task::getFd(fd));
    return __try(task::installFd(fdObj));
}

}  // namespace nyan::syscall
