#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int fcntl64(int fd, uint32_t request, uint32_t param) {
    auto fdobj = __try(task::getFd(fd));

    switch (request) {
        case F_SETFD:
            fdobj->__close_on_exec = param & FD_CLOEXEC;
            return 0;
        case F_GETFD:
            return fdobj->__close_on_exec ? FD_CLOEXEC : 0;
    }

    return SYS_ENOSYS;
}

}  // namespace nyan::syscall
