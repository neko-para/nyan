#include <nyan/syscall.h>

#include "../fs/load.hpp"
#include "../fs/vnode.hpp"
#include "../task/tcb.hpp"

namespace nyan::syscall {

int chdir(const char* pathname) {
    if (!pathname) {
        return -SYS_EFAULT;
    }
    // TODO: deal relative
    auto vnode = fs::resolve(pathname);
    if (!vnode) {
        return -SYS_ENOENT;
    }
    if (vnode->__type != fs::VNT_Directory) {
        return -SYS_ENOTDIR;
    }
    task::currentTask->cwd = pathname;
    return 0;
}

}  // namespace nyan::syscall