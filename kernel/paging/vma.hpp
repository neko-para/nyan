#pragma once

#include <stdint.h>
#include <sys/mman.h>
#include <compare>
#include <optional>
#include <vector>

#include "address.hpp"
#include "directory.hpp"
#include "range.hpp"

namespace nyan::paging {

struct VMA : public RangeEntryBase {
    uint32_t __flags{};
    uint32_t __protect{};
    std::string_view __name;

    VMA() = default;

    VMA(VirtualAddress begin, VirtualAddress end) {
        __begin = begin;
        __end = end;
    }

    VMA(VirtualAddress begin, VirtualAddress end, uint32_t flags, uint32_t prot, std::string_view name)
        : VMA(begin, end) {
        __flags = flags;
        __protect = prot;
        __name = name;
    }

    void alloc(UserDirectory& pageDir) noexcept {
        for (auto page = __begin; page != __end; page = page.nextPage()) {
            pageDir.ensure(page.tableLoc(), PDE_User | PDE_ReadWrite | PDE_Present);
            pageDir.with(page.tableLoc(), [&](Table* table) {
                auto attr = PTE_User | PTE_Present;
                if (__protect & PROT_WRITE) {
                    attr |= PTE_ReadWrite;
                }
                table->alloc(page, attr);
            });
        }
    }
};

struct VMSpace : public RangeManager<0x00400000, 0xC0000000, VMA> {
    using manager = RangeManager<0x00400000, 0xC0000000, VMA>;
    using iterator = manager::iterator;
    using const_iterator = manager::const_iterator;

    iterator find_name(std::string_view name) noexcept {
        return std::find_if(__addrs.begin(), __addrs.end(), [&](const VMA& vma) { return vma.__name == name; });
    }

    void __release_range(VirtualAddress begin, VirtualAddress end, UserDirectory& pageDir) noexcept {
        while (begin != end) {
            pageDir.freePage(begin);
            begin = begin.nextPage();
        }
    }

    void __split_range(VMA& entry, const VMA& from) noexcept {
        entry.__flags = from.__flags & (~MAP_GROWSDOWN);
        entry.__protect = from.__protect;
        entry.__name = from.__name;
    }

    bool erase(VirtualAddress addr, size_t size, UserDirectory& pageDir) {
        return __erase(
            addr, size,
            [&](VirtualAddress begin, VirtualAddress end) noexcept { __release_range(begin, end, pageDir); },
            [&](VMA& entry, const VMA& from) { __split_range(entry, from); });
    }

    void free(UserDirectory& pageDir) {
        for (const auto& vma : *this) {
            __release_range(vma.__begin, vma.__end, pageDir);
        }
        clear();
    }

    void dump() {
        arch::kprint("Dump VMSpace\n");
        for (const auto& vma : *this) {
            arch::kprint("{8}: {#10x} ~ {#10x} {x} {x}\n", vma.__name, vma.__begin.addr, vma.__end.addr, vma.__flags,
                         vma.__protect);
        }
    }
};

}  // namespace nyan::paging
