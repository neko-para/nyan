#include <nyan/syscall.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

int dup2(int fd, int newFd) {
    auto oldFdObj = __try(task::getFd(fd));
    if (fd == newFd) {
        return fd;
    }
    __try
        (task::installFdTo(oldFdObj, newFd));
    return newFd;
}

}  // namespace nyan::syscall
