#include "dentry.hpp"

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

}  // namespace nyan::fs
