#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../fs/vnode.hpp"
#include "../task/mod.hpp"
#include "utils.hpp"

namespace nyan::syscall {

int getdents64(int fd, struct dirent* dirents, unsigned count) {
    if (!utils::validateRead(dirents, count)) {
        return SYS_EFAULT;
    }

    auto fdobj = __try(task::getFd(fd));
    auto vnode = fdobj->__file->getVNode();
    if (!vnode || !vnode->isDirectory()) {
        return SYS_ENOTDIR;
    }
    return __try(vnode->readdir(dirents, count, &fdobj->__file->__offset));
}

}  // namespace nyan::syscall
