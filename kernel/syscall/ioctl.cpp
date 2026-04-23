#include <nyan/syscall.h>
#include <sys/ioctl.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

int ioctl(int fd, uint32_t request, uint32_t param) {
    if (fd < 0 || static_cast<size_t>(fd) >= task::MAXFD) {
        return -SYS_EBADF;
    }
    const auto& fileObj = task::currentTask->fdTable[fd];
    if (!fileObj) {
        return -SYS_EBADF;
    }
    return fileObj->ioctl(request, param);
}

}  // namespace nyan::syscall
