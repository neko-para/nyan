#include <nyan/syscall.h>

#include "../fs/fd.hpp"
#include "../fs/vnode.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int getdents64(int fd, struct dirent* dirent, unsigned count) {
    __try
        (task::checkAddr(dirent, count, PROT_WRITE));

    auto fdobj = __try(task::getFd(fd));
    auto vnode = fdobj->__file->getVNode();
    if (!vnode || !vnode->isDirectory()) {
        return SYS_ENOTDIR;
    }
    return __try(vnode->readdir(dirent, count, &fdobj->__file->__offset));
}

}  // namespace nyan::syscall
