#include <nyan/syscall.h>
#include <sys/ioctl.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int ioctl(int fd, uint32_t request, uint32_t param) {
    auto fileObj = task::__scheduler->__current->__file.getFile(fd);
    if (!fileObj) {
        return -SYS_EBADF;
    }
    return fileObj->ioctl(request, param);
}

}  // namespace nyan::syscall
