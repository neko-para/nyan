#pragma once

#include "../arch/print.hpp"
#include "../paging/range.hpp"
#include "map.hpp"

namespace nyan::allocator {

struct LFEntry : public paging::RangeEntryBase {
    using RangeEntryBase::RangeEntryBase;
};

struct LargeFrameManager : public paging::RangeManager<__large_frame_base, __large_frame_top, LFEntry> {
    std::optional<paging::VirtualAddress> alloc(size_t page) noexcept;
    uint32_t free(paging::VirtualAddress addr) noexcept;
};

}  // namespace nyan::allocator
