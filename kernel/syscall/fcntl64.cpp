#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int fcntl64(int fd, unsigned cmd, uint32_t arg) {
    auto fdObj = __try(task::getFd(fd));

    switch (cmd) {
        case F_DUPFD:
            return __try(task::installFd(fdObj, arg));
        case F_DUPFD_CLOEXEC:
            return __try(task::installFd(fdObj, arg, true));
        case F_SETFD:
            fdObj->__close_on_exec = arg & FD_CLOEXEC;
            return 0;
        case F_GETFD:
            return fdObj->__close_on_exec ? FD_CLOEXEC : 0;
    }

    return SYS_ENOSYS;
}

}  // namespace nyan::syscall
