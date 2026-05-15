#include "mod.hpp"

#include <fcntl.h>
#include <nyan/errno.h>

#include "../arch/guard.hpp"
#include "../task/mod.hpp"
#include "dentry.hpp"
#include "mount.hpp"
#include "vnode.hpp"
#include "vnode_file.hpp"

namespace nyan::fs {

Result<lib::Ref<FileObj>> open(std::string_view __path, uint32_t flags, uint32_t mode) {
    arch::InterruptGuard guard;

    Path path{__path};
    if (path.__invalid) {
        return SYS_ENOENT;
    }

    if (path.__portions.empty()) {  // '.' or '/'
        auto entry = __try(resolve(path, {}));
        if (flags & O_CREAT) {
            return SYS_EEXIST;
        }
        auto vnode = entry->effectiveVNode();
        return __try(vnode->open(vnode, flags & O_ACCMODE));
    }

    Path upper = path.parent();

    auto parentEntry = __try(resolve(upper, {}));
    if (!parentEntry) {
        return SYS_ENOENT;
    }

    auto entry = parentEntry->effectiveVNode()->lookup(path.last());
    if (!entry) {
        if ((flags & O_CREAT) && !path.__trailing_slash) {
            __try
                (parentEntry->effectiveVNode()->create(path.last(), mode));
        } else {
            return SYS_ENOENT;
        }
    } else {
        if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
            return SYS_EEXIST;
        }
    }

    auto vnode = __try(parentEntry->effectiveVNode()->lookup(path.last()));
    if (vnode->isSymlink() && !(flags & O_NOFOLLOW)) {
        auto target = __try(vnode->readlink());
        auto newEntry = __try(resolve(Path{target}, {}));
        vnode = newEntry->effectiveVNode();
    }

    if (((flags & O_DIRECTORY) || path.__trailing_slash) && !vnode->isDirectory()) {
        return SYS_ENOTDIR;
    }
    if ((flags & O_ACCMODE) != O_RDONLY && flags & O_TRUNC && vnode->isRegular()) {
        __try
            (vnode->truncate(0));
    }
    return __try(vnode->open(vnode, flags & (O_ACCMODE | O_APPEND)));
}

Result<lib::Ref<DEntry>> resolve(const Path& path, ResolveConfig config, int loop) {
    if (loop >= 10) {
        return SYS_ELOOP;
    }

    if (path.__invalid) {
        return SYS_ENOENT;
    }

    lib::Ref<DEntry> current;
    if (path.__relative) {
        if (config.__from) {
            current = config.__from;
        } else {
            current = task::getCwd();
        }
        if (!current) {
            return SYS_ENOENT;
        }
    } else {
        current = (*mountPoints)[0]->__mount_point;
    }

    for (auto it = path.__portions.begin(); it != path.__portions.end(); it++) {
        const auto& portion = *it;
        bool last = std::next(it) == path.__portions.end();

        if (portion == "..") {
            if (current->__parent) {
                current = current->__parent;
            }
        } else {
            auto next = __try(dentryCacheManager->lookup(current, portion));
            if ((config.__follow || !last) && next->effectiveVNode()->isSymlink()) {
                auto target = __try(next->effectiveVNode()->readlink());
                auto resolvedNext = __try(resolve({target}, {true, current}, loop + 1));
                if (!resolvedNext) {
                    return SYS_ENOENT;
                }
                next = resolvedNext;
            }
            current = next;
        }
    }
    return current;
}

}  // namespace nyan::fs
