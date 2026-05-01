#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../fs/mount.hpp"
#include "../fs/vnode.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int chdir(const char* pathname) {
    if (!pathname) {
        return -SYS_EFAULT;
    }
    // TODO: deal relative
    auto dentry = fs::resolve(pathname);
    if (!dentry || !dentry->__node) {
        return -SYS_ENOENT;
    }
    if (dentry->__node->__type != fs::VNT_Directory) {
        return -SYS_ENOTDIR;
    }
    task::currentTask->cwd = dentry;
    return 0;
}

}  // namespace nyan::syscall