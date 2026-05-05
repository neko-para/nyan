#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../fs/fd.hpp"
#include "../fs/vnode.hpp"
#include "../fs/vnode_file.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int open(const char* pathname, int flags, mode_t mode) {
    auto entry = fs::resolve(pathname);
    if (!entry) {
        return -SYS_ENOENT;
    }

    auto vnode = entry->effectiveVNode();
    auto file = vnode->open(vnode, flags & O_ACCMODE);

    int fd;
    auto fdObjPtr = task::__scheduler->__current->__file.getFileSlot(fd);
    if (!fdObjPtr) {
        return -SYS_EMFILE;
    }

    *fdObjPtr = lib::makeRef<fs::FdObj>(file);
    return fd;
}

}  // namespace nyan::syscall