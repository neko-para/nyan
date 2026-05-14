#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../fs/mod.hpp"
#include "../fs/vnode.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

ssize_t readlink(const char* __path, char* buf, size_t size) {
    if (size == 0) {
        return SYS_EINVAL;
    }

    auto path = __try(task::checkString(__path));
    __try
        (task::checkW(buf, size));

    auto dentry = __try(fs::resolve({path}, {false}));
    auto vnode = dentry->effectiveVNode();

    auto target = __try(vnode->readlink());
    if (size < target.size()) {
        return SYS_ERANGE;
    }
    memcpy(buf, target.c_str(), target.size());
    return target.size();
}

}  // namespace nyan::syscall
