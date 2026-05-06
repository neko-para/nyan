#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../fs/vnode.hpp"
#include "utils.hpp"

namespace nyan::syscall {

int chdir(const char* pathname) {
    auto path = utils::validateString(pathname);
    if (!path) {
        return -SYS_EFAULT;
    }
    auto [dentry, _1, _2] = fs::resolve(*path);
    if (!dentry || !dentry->__node) {
        return -SYS_ENOENT;
    }
    if (dentry->__node->__type != fs::VNT_Directory) {
        return -SYS_ENOTDIR;
    }
    utils::setCwd(dentry);
    return 0;
}

}  // namespace nyan::syscall
