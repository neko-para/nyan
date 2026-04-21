#include <nyan/syscall.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

int close(int fd) {
    if (fd < 0 || static_cast<size_t>(fd) >= task::MAXFD) {
        return -SYS_EBADF;
    }
    auto& fileObj = task::currentTask->fdTable[fd];
    if (!fileObj) {
        return -SYS_EBADF;
    }
    fileObj = {};
    return 0;
}

}  // namespace nyan::syscall
