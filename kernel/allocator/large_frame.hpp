#pragma once

#include "../arch/print.hpp"
#include "../paging/range.hpp"
#include "map.hpp"

namespace nyan::allocator {

struct LFEntry : public paging::RangeEntryBase {
    using RangeEntryBase::RangeEntryBase;
};

struct LargeFrameManager : public paging::RangeManager<largeFrameBase, largeFrameTop, LFEntry> {
    std::optional<paging::VirtualAddress> alloc(size_t page) {
        if (auto addr = find_free(page << 12)) {
            LFEntry entry{*addr, *addr + (page << 12)};
            if (!insert(entry)) {
                return std::nullopt;
            }
            return *addr;
        } else {
            return std::nullopt;
        }
    }

    uint32_t free(paging::VirtualAddress addr) {
        if (addr.thisPage() != addr) {
            arch::kfatal("free addr not aligned");
        }

        auto loc = __locate(addr);
        if (loc->__begin != addr) {
            arch::kfatal("free addr not the begining");
        }

        auto page = static_cast<uint32_t>(loc->__end - loc->__begin) >> 12;
        __erase(loc);
        return page;
    }
};

}  // namespace nyan::allocator
