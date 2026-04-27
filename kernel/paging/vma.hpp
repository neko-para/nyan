#pragma once

#include <stdint.h>
#include <sys/mman.h>
#include <compare>
#include <optional>
#include <vector>

#include "address.hpp"

namespace nyan::paging {

struct VMA {
    VirtualAddress __begin;
    VirtualAddress __end;
    uint32_t __flags;
    uint32_t __protect;

    bool contains(VirtualAddress addr) const noexcept { return __begin <= addr && addr < __end; }
};

struct VMSpace {
    std::vector<VMA> __addrs;

    std::vector<VMA>::iterator find(VirtualAddress addr, std::vector<VMA>::iterator& pos) noexcept {
        pos = std::upper_bound(__addrs.begin(), __addrs.end(), addr,
                               [](const VirtualAddress& addr, const VMA& item) { return addr < item.__end; });

        if (pos == __addrs.end()) {
            return pos;
        }

        if (pos->contains(addr)) {
            return pos;
        } else {
            return __addrs.end();
        }
    }

    std::vector<VMA>::iterator find_exactly(VirtualAddress addr) noexcept {
        auto pos = std::lower_bound(__addrs.begin(), __addrs.end(), addr,
                                    [](const VMA& item, const VirtualAddress& addr) { return addr > item.__begin; });

        if (pos == __addrs.end()) {
            return pos;
        }

        if (pos->__begin == addr) {
            return pos;
        } else {
            return __addrs.end();
        }
    }

    bool insert(const VMA& vma) noexcept {
        auto pos = std::lower_bound(__addrs.begin(), __addrs.end(), vma,
                                    [](const VMA& item, const VMA& vma) { return item.__begin < vma.__begin; });

        if (pos != __addrs.begin()) {
            auto prev = std::prev(pos);
            if (prev->__end > vma.__begin) {
                return false;
            }
        }

        if (pos != __addrs.end()) {
            if (vma.__end > pos->__begin) {
                return false;
            }
        }

        __addrs.insert(pos, vma);
        return true;
    }
};

}  // namespace nyan::paging
