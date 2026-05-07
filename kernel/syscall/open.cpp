#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../fs/mod.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"
#include "utils.hpp"

namespace nyan::syscall {

int open(const char* pathname, int flags, mode_t mode) {
    auto path = utils::validateString(pathname);
    if (!path) {
        return SYS_EFAULT;
    }

    auto file = __try(fs::open(*path, flags, mode));

    int fd;
    auto fdObjPtr = __try(task::__scheduler->__current->__file.findFileSlot(fd));

    *fdObjPtr = lib::makeRef<fs::FdObj>(file);
    if (flags & O_CLOEXEC) {
        (*fdObjPtr)->__close_on_exec = true;
    }
    return fd;
}

}  // namespace nyan::syscall
