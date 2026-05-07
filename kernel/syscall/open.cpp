#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../fs/mod.hpp"
#include "../task/mod.hpp"
#include "utils.hpp"

namespace nyan::syscall {

int open(const char* pathname, int flags, mode_t mode) {
    auto path = utils::validateString(pathname);
    if (!path) {
        return SYS_EFAULT;
    }

    auto file = __try(fs::open(*path, flags, mode));

    auto [fd, fdObj] = __try(task::installFile(file));
    if (flags & O_CLOEXEC) {
        fdObj->__close_on_exec = true;
    }
    return fd;
}

}  // namespace nyan::syscall
