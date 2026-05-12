#include <nyan/syscall.h>

#include "../task/mod.hpp"

namespace nyan::syscall {

int dup(int fildes) {
    auto fdObj = __try(task::getFd(fildes));
    return __try(task::installFd(fdObj));
}

}  // namespace nyan::syscall
