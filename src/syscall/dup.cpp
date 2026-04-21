#include <nyan/syscall.h>

#include "../arch/guard.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int dup(int fd) {
    if (fd < 0 || static_cast<size_t>(fd) >= task::MAXFD) {
        return -SYS_EBADF;
    }
    auto& fileObj = task::currentTask->fdTable[fd];
    if (!fileObj) {
        return -SYS_EBADF;
    }

    for (size_t i = 3; i < task::MAXFD; i++) {
        auto& fileObjSlot = task::currentTask->fdTable[fd];
        if (fileObjSlot) {
            continue;
        }
        fileObjSlot = fileObj;
        return i;
    }
    return -SYS_EMFILE;
}

}  // namespace nyan::syscall
