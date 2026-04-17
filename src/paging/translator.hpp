#pragma once

#include "../lib/containers.hpp"
#include "address.hpp"
#include "directory.hpp"

namespace nyan::paging {

struct Translator {
    struct Entry {
        MapperGuard mapper;
        VirtualAddress user;
    };

    lib::vector<Entry> entries;

    Translator() = default;
    Translator(const Translator&) = delete;
    Translator(Translator&&) = default;
    ~Translator() = default;
    Translator& operator=(const Translator&) = delete;
    Translator& operator=(Translator&&) = default;

    void addEntry(MapperGuard mapper, VirtualAddress user) { entries.push_back(Entry{std::move(mapper), user}); }

    VirtualAddress toUser(VirtualAddress addr) const {
        for (const auto& entry : entries) {
            if (entry.mapper.vaddr == addr.thisPage()) {
                return entry.user + addr.pageOffset();
            }
        }
        arch::kfatal("no translator entry for addr {#10x}", addr.addr);
    }
};

}  // namespace nyan::paging
