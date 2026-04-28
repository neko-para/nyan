#pragma once

#include <stdint.h>
#include <sys/mman.h>
#include <compare>
#include <optional>
#include <vector>

#include "address.hpp"
#include "directory.hpp"

namespace nyan::paging {

struct VMA {
    VirtualAddress __begin;
    VirtualAddress __end;
    uint32_t __flags;
    uint32_t __protect;

    std::string_view __name;

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

    std::vector<VMA>::const_iterator find(VirtualAddress addr, std::vector<VMA>::const_iterator& pos) const noexcept {
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

    std::optional<VirtualAddress> find_free(size_t size, VirtualAddress hint = 0x40000000_va) const noexcept {
        if (hint >= 0xC0000000_va) {
            return std::nullopt;
        } else if (hint < 0x00400000_va) {
            hint = 0x00400000_va;
        }
        std::vector<VMA>::const_iterator next;
        auto pos = find(hint, next);
        if (pos == __addrs.end()) {
            if (next != __addrs.begin()) {
                pos = std::prev(next);
            } else {
                pos = next;
            }
        }
        while (pos != __addrs.end()) {
            if (pos->__end >= 0xC0000000_va) {
                return std::nullopt;
            }

            auto next2 = std::next(pos);
            if (next2 == __addrs.end()) {
                // 一定会被stack夹住, 不可能没有next
                return std::nullopt;
            }

            auto rest = next2->__begin - pos->__end;
            if (rest >= size) {
                return pos->__end;
            } else {
                pos = next2;
            }
        }
        return std::nullopt;
    }

    void __release_range(VirtualAddress begin, VirtualAddress end, UserDirectory& pageDir) {
        while (begin != end) {
            pageDir.freePage(begin);
            begin = begin.nextPage();
        }
    }

    bool erase(VirtualAddress addr, size_t size, UserDirectory& pageDir) {
        std::vector<VMA>::iterator next;
        auto left = find(addr, next);
        if (left == __addrs.end()) {
            if (next == __addrs.end()) {
                return true;
            }
            left = next;
        }

        auto right = find(addr + size - 1, next);
        if (right == __addrs.end()) {
            if (next == __addrs.begin()) {
                return true;
            }
            right = std::prev(next);
        }

        auto lower = addr;
        auto upper = addr + size;

        if (left > right) {
            return true;
        } else if (left == right) {
            // 仅影响这一页
            if (left->__begin >= lower) {
                if (left->__end <= upper) {
                    __release_range(left->__begin, left->__end, pageDir);
                    __addrs.erase(left);
                    return true;
                } else {
                    __release_range(left->__begin, upper, pageDir);
                    left->__begin = upper;
                    return true;
                }
            } else if (left->__end <= upper) {
                __release_range(lower, left->__end, pageDir);
                left->__end = lower;
                return true;
            } else {
                __release_range(lower, upper, pageDir);
                VMA vma = {
                    left->__begin, lower, left->__flags & (~MAP_GROWSDOWN), left->__protect, left->__name,
                };
                left->__begin = upper;
                __addrs.insert(left, vma);
                return true;
            }
        } else {
            // 影响若干页
            auto first_to_remove = left;
            auto last_to_remove = right;
            if (left->__begin < lower) {
                __release_range(lower, left->__end, pageDir);
                left->__end = lower;
                first_to_remove = std::next(first_to_remove);
            }

            if (right->__end > upper) {
                __release_range(right->__begin, upper, pageDir);
                right->__begin = upper;
                last_to_remove = std::prev(right);
            }

            if (first_to_remove <= last_to_remove) {
                last_to_remove = std::next(last_to_remove);
                for (auto it = first_to_remove; it != last_to_remove; it++) {
                    __release_range(it->__begin, it->__end, pageDir);
                }
                __addrs.erase(first_to_remove, last_to_remove);
            }

            return true;
        }
    }

    void free(UserDirectory& pageDir) {
        for (const auto& vma : __addrs) {
            __release_range(vma.__begin, vma.__end, pageDir);
        }
        __addrs.clear();
    }

    void dump() {
        arch::kprint("Dump VMSpace\n");
        for (const auto& vma : __addrs) {
            arch::kprint("{8}: {#10x} ~ {#10x} {x} {x}\n", vma.__name, vma.__begin.addr, vma.__end.addr, vma.__flags,
                         vma.__protect);
        }
    }
};

}  // namespace nyan::paging
