#include "dentry.hpp"

#include <ranges>

#include "../task/scheduler.hpp"
#include "../task/tcb.hpp"
#include "load.hpp"
#include "mount.hpp"
#include "vnode.hpp"

namespace nyan::fs {

static uint64_t __counter;
DEntryCacheManager* dentryCacheManager;

DEntry::DEntry(lib::Ref<DEntry> parent, lib::Ref<VNode> node, std::string name) noexcept
    : __parent(parent), __node(node), __name(std::move(name)) {}

void DEntry::onRefInc() noexcept {
    if (__ref_count == 2) {
        dentryCacheManager->__unused_list.erase({this});
    } else if (__ref_count == 1) {
        dentryCacheManager->__unused_list.push_front(this);
    }
}

void DEntry::onRefDec() noexcept {
    if (__ref_count == 1) {
        dentryCacheManager->__unused_list.push_front(this);
    } else if (__ref_count == 0) {
        dentryCacheManager->__unused_list.erase({this});
    }
}

Result<lib::Ref<DEntry>> DEntryCacheManager::lookup(lib::Ref<DEntry> parent, std::string_view name) noexcept {
    DEntryCache cache_key = {parent->__id, std::string{name}};
    if (auto it = __cache.find(cache_key); it != __cache.end()) {
        return it->second;
    } else {
        auto target_vnode = parent->effectiveVNode();
        auto next = __try(target_vnode->lookup(name));
        auto dentry = lib::makeRef<DEntry>(parent, next, std::string{name});
        dentry->__id = allocDEntryId();
        __cache.emplace(std::move(cache_key), dentry);
        return dentry;
    }
}

uint64_t allocDEntryId() {
    return ++__counter;
}

DEntryResolveResult resolve(std::string_view path) {
    if (path.empty()) {
        return {};
    }

    auto current = rootEntry()->__mount_point;
    if (path[0] != '/') {
        current = task::__scheduler->__current->cwd;
        if (!current) {
            return {};
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
            auto next = dentryCacheManager->lookup(current, portion);
            if (!next) {
                bool isLast = std::next(it) == portions.end();
                if (isLast) {
                    return {
                        nullptr,
                        current,
                        std::string{portion},
                    };
                } else {
                    return {};
                }
            }
            current = *next;
        }
    }
    return {
        current,
        current->__parent,
        std::string{portions.back()},
    };
}

}  // namespace nyan::fs
