#include "large_frame.hpp"

namespace nyan::allocator {

std::optional<paging::VirtualAddress> LargeFrameManager::alloc(size_t page) noexcept {
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

uint32_t LargeFrameManager::free(paging::VirtualAddress addr) noexcept {
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

}  // namespace nyan::allocator