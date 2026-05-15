#include <nyan/errno.h>
#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../fs/file.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int fstat64(int fd, struct stat* statbuf) {
    __try
        (task::checkW(statbuf));

    auto fdObj = __try(task::getFd(fd));
    __try
        (fdObj->__file->stat(statbuf));

    return 0;
}

}  // namespace nyan::syscall
