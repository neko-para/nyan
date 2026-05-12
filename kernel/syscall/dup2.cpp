#include <nyan/syscall.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

int dup2(int oldfd, int newfd) {
    auto oldFdObj = __try(task::getFd(oldfd));
    if (oldfd == newfd) {
        return oldfd;
    }
    __try
        (task::installFdTo(oldFdObj, newfd));
    return newfd;
}

}  // namespace nyan::syscall
