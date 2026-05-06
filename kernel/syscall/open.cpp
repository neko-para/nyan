#include <fcntl.h>
#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../fs/fd.hpp"
#include "../fs/vnode.hpp"
#include "../fs/vnode_file.hpp"
#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"
#include "utils.hpp"

namespace nyan::syscall {

int open(const char* pathname, int flags, mode_t mode) {
    auto path = utils::validateString(pathname);
    if (!path) {
        return -SYS_EFAULT;
    }
    auto [entry, parent, name] = fs::resolve(*path);
    lib::Ref<fs::VNode> vnode;
    if (!entry) {
        if ((flags & O_CREAT) && parent && !name.empty()) {
            int ret = parent->effectiveVNode()->create(name, mode);
            if (ret) {
                return ret;
            }
            vnode = parent->effectiveVNode()->lookup(name);
        } else {
            return -SYS_ENOENT;
        }
    } else {
        if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
            return -SYS_EEXIST;
        }
        vnode = entry->effectiveVNode();
    }
    if ((flags & O_DIRECTORY) && vnode->__type != fs::VNT_Directory) {
        return -SYS_ENOTDIR;
    }
    if ((flags & O_ACCMODE) != O_RDONLY && flags & O_TRUNC && vnode->__type == fs::VNT_Regular) {
        int ret = vnode->truncate(0);
        if (ret) {
            return ret;
        }
    }
    auto file = vnode->open(vnode, flags & O_ACCMODE);

    int fd;
    auto fdObjPtr = task::__scheduler->__current->__file.findFileSlot(fd);
    if (!fdObjPtr) {
        return -SYS_EMFILE;
    }

    *fdObjPtr = lib::makeRef<fs::FdObj>(file);
    if (flags & O_CLOEXEC) {
        (*fdObjPtr)->__close_on_exec = true;
    }
    return fd;
}

}  // namespace nyan::syscall
