#include <nyan/syscall.h>

#include "../fs/dentry.hpp"
#include "../fs/mod.hpp"
#include "../fs/vnode.hpp"
#include "../task/mod.hpp"

namespace nyan::syscall {

int link(const char* __oldname, const char* __newname) {
    auto oldname = __try(task::checkString(__oldname));
    auto newname = __try(task::checkString(__newname));

    auto oldEntry = __try(fs::resolve({oldname}, {}));

    fs::Path newPath{__newname};
    if (newPath.__invalid) {
        return SYS_ENOENT;
    }

    if (newPath.__portions.empty()) {
        return SYS_EEXIST;
    }

    auto upperPath = newPath.parent();

    auto parentEntry = __try(fs::resolve(upperPath, {}));

    auto entry = parentEntry->effectiveVNode()->lookup(newPath.last());
    if (entry) {
        return SYS_EEXIST;
    } else if (entry != SYS_ENOENT) {
        return entry.error();
    }

    __try
        (parentEntry->effectiveVNode()->link(newPath.last(), oldEntry->effectiveVNode()));

    return 0;
}

}  // namespace nyan::syscall
