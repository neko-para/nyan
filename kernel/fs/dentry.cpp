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

lib::Ref<DEntry> DEntryCacheManager::lookup(lib::Ref<DEntry> parent, std::string_view name) noexcept {
    DEntryCache cache_key = {parent->__id, std::string{name}};
    if (auto it = __cache.find(cache_key); it != __cache.end()) {
        return it->second;
    } else {
        auto target_vnode = parent->effectiveVNode();
        auto next = target_vnode->lookup(name);
        if (!next) {
            return {};
        }
        auto dentry = lib::makeRef<DEntry>(parent, next, std::string{name});
        dentry->__id = allocDEntryId();
        __cache.emplace(std::move(cache_key), dentry);
        return dentry;
    }
}

uint64_t allocDEntryId() {
    return ++__counter;
}

lib::Ref<DEntry> resolve(std::string_view path) {
    if (path.empty()) {
        return {};
    }

    auto current = lib::makeRef<DEntry>(nullptr, rootEntry()->__root_node, "");
    if (path[0] != '/') {
        current = task::__scheduler->__current->cwd;
        if (!current) {
            return {};
        }
    }

    for (const auto& item : path | std::views::split('/')) {
        std::string_view portion{item.begin(), item.end()};
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
                return {};
            }
            current = next;
        }
    }
    return current;
}

}  // namespace nyan::fs