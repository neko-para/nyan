#include <nyan/syscall.h>
#include <sys/ioctl.h>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int ioctl(int fd, uint32_t request, uint32_t param) {
    return __try(task::__scheduler->__current->__file.getFile(fd))->ioctl(request, param).merge();
}

}  // namespace nyan::syscall
