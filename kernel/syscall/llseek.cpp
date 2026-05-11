#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int llseek(int fd, uint32_t offset_high, uint32_t offset_low, loff_t* result, int whence) {
    __try
        (task::checkW(result));

    if (offset_high >= 0x80000000) {
        return SYS_EINVAL;
    }

    auto fdobj = __try(task::getFd(fd));
    *result = __try(fdobj->seek((static_cast<off_t>(offset_high) << 32) | offset_low, whence));
    return 0;
}

}  // namespace nyan::syscall
