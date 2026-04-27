#pragma once

#include <vector>

#include "address.hpp"
#include "directory.hpp"

namespace nyan::paging {

struct Translator {
    struct Entry {
        MapperGuard mapper;
        VirtualAddress user;
    };

    std::vector<Entry> entries;

    Translator() = default;
    Translator(const Translator&) = delete;
    Translator(Translator&&) = default;
    ~Translator() = default;
    Translator& operator=(const Translator&) = delete;
    Translator& operator=(Translator&&) = default;

    template <typename T = void>
    T* addEntry(MapperGuard mapper, VirtualAddress user) {
        auto ptr = mapper.as<T>();
        entries.push_back(Entry{std::move(mapper), user});
        return ptr;
    }

    template <typename T = void>
    T* allocEntry(UserDirectory& pageDir, VirtualAddress user, bool writable) {
        return addEntry<T>(pageDir.alloc(user, writable), user);
    }

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
