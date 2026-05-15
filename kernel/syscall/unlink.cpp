#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../fs/mod.hpp"
#include "../fs/vnode.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int unlink(const char* __pathname) {
    auto pathname = __try(task::checkString(__pathname));

    auto entry = __try(fs::resolve({pathname}, {}));

    if (entry->effectiveVNode()->isDirectory()) {
        return SYS_EISDIR;
    }

    __try
        (entry->__parent->effectiveVNode()->unlink(entry->__name));

    return 0;
}

}  // namespace nyan::syscall
