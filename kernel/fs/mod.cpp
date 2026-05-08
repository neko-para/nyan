#include "mod.hpp"

#include <fcntl.h>
#include <nyan/errno.h>
#include <ranges>

#include "../task/mod.hpp"
#include "dentry.hpp"
#include "mount.hpp"
#include "vnode.hpp"
#include "vnode_file.hpp"

namespace nyan::fs {

Result<lib::Ref<FileObj>> open(std::string_view path, uint32_t flags, uint32_t mode) {
    lib::Ref<DEntry> parent;
    std::string name;
    auto entry = __try(resolveParent(path, &parent, &name));
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
    if ((flags & O_DIRECTORY) && !vnode->isDirectory()) {
        return SYS_ENOTDIR;
    }
    if ((flags & O_ACCMODE) != O_RDONLY && flags & O_TRUNC && vnode->isRegular()) {
        __try
            (vnode->truncate(0));
    }
    // TODO: result support cast
    return __try(vnode->open(vnode, flags & O_ACCMODE));
}

template <bool CreateMode>
static Result<lib::Ref<DEntry>> __resolveImpl(std::string_view path, lib::Ref<DEntry>* parent, std::string* lastName) {
    if (path.empty()) {
        return SYS_ENOENT;
    }

    auto current = rootEntry()->__mount_point;
    if (path[0] != '/') {
        current = task::getCwd();
        if (!current) {
            return SYS_ENOENT;
        }
    }

    std::vector<std::string_view> portions;
    for (const auto& item : path | std::views::split('/')) {
        portions.push_back({item.begin(), item.end()});
    }

    for (auto it = portions.begin(); it != portions.end(); it++) {
        auto portion = *it;
        if (portion.empty()) {
            continue;
        } else if (portion == ".") {
            continue;
        } else if (portion == "..") {
            if (current->__parent) {
                current = current->__parent;
            }
        } else {
            if constexpr (CreateMode) {
                auto next = dentryCacheManager->lookup(current, portion);
                if (!next) {
                    if (next == SYS_ENOENT) {
                        bool isLast = std::next(it) == portions.end();
                        if (isLast) {
                            if (parent) {
                                *parent = current;
                            }
                            if (lastName) {
                                *lastName = std::string{portion};
                            }
                        }
                        return {};
                    } else {
                        return next.error();
                    }
                }
                current = *next;
            } else {
                auto next = __try(dentryCacheManager->lookup(current, portion));
                current = next;
            }
        }
    }
    if constexpr (CreateMode) {
        if (parent) {
            *parent = current->__parent;
        }
        if (lastName) {
            *lastName = std::string{portions.back()};
        }
    }
    return current;
}

Result<lib::Ref<DEntry>> resolve(std::string_view path) {
    return __resolveImpl<false>(path, nullptr, nullptr);
}

Result<lib::Ref<DEntry>> resolveParent(std::string_view path, lib::Ref<DEntry>* parent, std::string* lastName) {
    return __resolveImpl<true>(path, parent, lastName);
}

}  // namespace nyan::fs
