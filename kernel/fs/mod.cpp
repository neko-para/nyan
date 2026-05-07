#include "mod.hpp"

#include <fcntl.h>
#include <nyan/errno.h>

#include "dentry.hpp"
#include "vnode.hpp"
#include "vnode_file.hpp"

namespace nyan::fs {

Result<lib::Ref<FileObj>> open(std::string_view path, uint32_t flags, uint32_t mode) {
    auto [entry, parent, name] = resolve(path);
    lib::Ref<fs::VNode> vnode;
    if (!entry) {
        if ((flags & O_CREAT) && parent && !name.empty()) {
            __try
                (parent->effectiveVNode()->create(name, mode));
            vnode = __try(parent->effectiveVNode()->lookup(name));
        } else {
            return SYS_ENOENT;
        }
    } else {
        if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
            return SYS_EEXIST;
        }
        vnode = entry->effectiveVNode();
    }
    if ((flags & O_DIRECTORY) && vnode->__type != VNT_Directory) {
        return SYS_ENOTDIR;
    }
    if ((flags & O_ACCMODE) != O_RDONLY && flags & O_TRUNC && vnode->__type == VNT_Regular) {
        __try
            (vnode->truncate(0));
    }
    // TODO: result support cast
    return __try(vnode->open(vnode, flags & O_ACCMODE));
}

}  // namespace nyan::fs
