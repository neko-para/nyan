#include <nyan/syscall.h>
#include <sys/ioctl.h>

#include "../task/tcb.hpp"

namespace nyan::syscall {

int ioctl(int fd, uint32_t request, uint32_t param) {
    auto fileObjPtr = task::currentTask->__file.getFile(fd);
    if (!fileObjPtr) {
        return -SYS_EBADF;
    }
    return (*fileObjPtr)->ioctl(request, param);
}

}  // namespace nyan::syscall
