#pragma once

#include <compare>
#include <map>
#include <string>

#include "../lib/list.hpp"
#include "../lib/result.hpp"
#include "../lib/shared.hpp"
#include "forward.hpp"
#include "mount.hpp"

namespace nyan::fs {

struct DEntryLRUTag {
    using type = DEntry;
};

struct DEntry : public lib::Shared, public lib::ListNodes<DEntryLRUTag> {
    uint64_t __id;
    lib::Ref<DEntry> __parent;
    lib::Ref<VNode> __node;
    std::string __name;
    lib::Ref<MountEntry> __mount;

    DEntry() = default;
    DEntry(lib::Ref<DEntry> parent, lib::Ref<VNode> node, std::string name) noexcept;

    virtual void onRefInc() noexcept override;
    virtual void onRefDec() noexcept override;

    std::string asPath() const noexcept {
        if (!__parent) {
            return "/";
        } else if (!__parent->__parent) {
            return "/" + __name;
        } else {
            return __parent->asPath() + "/" + __name;
        }
    }
    lib::Ref<VNode> effectiveVNode() const noexcept { return __mount ? __mount->__root_node : __node; }
};

struct DEntryCache {
    uint64_t __parent_id;
    std::string __name;

    auto operator<=>(const DEntryCache&) const noexcept = default;
};

struct DEntryCacheManager {
    // 想办法换成hash
    std::map<DEntryCache, lib::Ref<DEntry>> __cache;
    lib::List<DEntryLRUTag, true> __unused_list;

    Result<lib::Ref<DEntry>> lookup(lib::Ref<DEntry> parent, std::string_view name) noexcept;
};

extern DEntryCacheManager* dentryCacheManager;

uint64_t allocDEntryId();

}  // namespace nyan::fs
