#include <nyan/syscall.h>
#include <sys/ioctl.h>

#include "../fs/fd.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int ioctl(int fd, unsigned cmd, uint32_t arg) {
    return __try(task::getFd(fd))->ioctl(cmd, arg).merge();
}

}  // namespace nyan::syscall
