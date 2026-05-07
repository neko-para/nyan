#include <nyan/syscall.h>
#include <sys/ioctl.h>

#include "../fs/fd.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int ioctl(int fd, uint32_t request, uint32_t param) {
    return __try(task::getFd(fd))->ioctl(request, param).merge();
}

}  // namespace nyan::syscall
