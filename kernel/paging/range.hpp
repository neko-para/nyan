#pragma once

#include <variant>
#include <vector>

#include "address.hpp"

namespace nyan::paging {

template <typename Entry>
concept range_entry_like = requires(Entry e, VirtualAddress addr) {
    { e.__begin } -> std::convertible_to<VirtualAddress>;
    { e.__end } -> std::convertible_to<VirtualAddress>;
    { e.contains(addr) } -> std::convertible_to<bool>;
    { e.bounds(e, e) } -> std::convertible_to<bool>;
    { e.bounds(e, addr) } -> std::convertible_to<bool>;
} && std::is_constructible_v<Entry, VirtualAddress, VirtualAddress>;

struct RangeEntryBase {
    VirtualAddress __begin;
    VirtualAddress __end;

    bool contains(VirtualAddress addr) const noexcept { return __begin <= addr && addr < __end; }
    template <typename Entry>
    bool bounds(const Entry& right, const Entry& target) const noexcept {
        return __end <= target.__begin && target.__end <= right.__begin;
    }
    template <typename Entry>
    bool bounds(const Entry& right, const VirtualAddress& addr) const noexcept {
        return __end <= addr && addr < right.__begin;
    }
};

template <uint32_t Base, uint32_t Top, range_entry_like RangeEntry>
struct RangeManager {
    constexpr static paging::VirtualAddress BaseAddr = paging::VirtualAddress{Base};
    constexpr static paging::VirtualAddress TopAddr = paging::VirtualAddress{Top};

    using iterator = std::vector<RangeEntry>::iterator;
    using const_iterator = std::vector<RangeEntry>::const_iterator;

    std::vector<RangeEntry> __addrs;

    RangeManager() {
        __addrs.push_back(RangeEntry{0_va, BaseAddr});
        __addrs.push_back(RangeEntry{TopAddr, 0xFFFFF000_va});
    }

    auto begin() noexcept { return std::next(__addrs.begin()); }

    auto begin() const noexcept { return std::next(__addrs.begin()); }

    auto end() noexcept { return std::prev(__addrs.end()); }

    auto end() const noexcept { return std::prev(__addrs.end()); }

    bool empty() const noexcept { return __addrs.size() == 2; }

    void clear() noexcept { __addrs.erase(begin(), end()); }

    iterator __locate(VirtualAddress addr) noexcept {
        auto it = std::partition_point(__addrs.begin(), __addrs.end(),
                                       [&](const RangeEntry& entry) noexcept { return !(entry.__begin > addr); });
        return std::prev(it);
    }

    const_iterator __locate(VirtualAddress addr) const noexcept {
        auto it = std::partition_point(__addrs.begin(), __addrs.end(),
                                       [&](const RangeEntry& entry) noexcept { return !(entry.__begin > addr); });
        return std::prev(it);
    }

    bool insert(const RangeEntry& entry) noexcept {
        auto pos = __locate(entry.__begin);
        auto right = std::next(pos);
        if (!pos->bounds(*right, entry)) {
            return false;
        }
        __addrs.insert(right, entry);
        return true;
    }

    std::optional<VirtualAddress> find_free(size_t size, VirtualAddress hint = BaseAddr) const noexcept {
        if (hint >= TopAddr) {
            return std::nullopt;
        } else if (hint < BaseAddr) {
            hint = BaseAddr;
        }

        auto pos = __locate(hint);
        auto right = std::next(pos);
        while (right != __addrs.end()) {
            uint32_t lower = std::max(pos->__end.addr, hint.addr);
            uint32_t upper = 0;
            if (__builtin_add_overflow(lower, size, &upper)) {
                return std::nullopt;
            }

            RangeEntry entry = {
                paging::VirtualAddress{lower},
                paging::VirtualAddress{upper},
            };
            if (pos->bounds(*right, entry)) {
                return entry.__begin;
            }
            pos = right;
            right = std::next(right);
        }
        return std::nullopt;
    }

    template <typename ReleaseFunc, typename SplitFunc>
    bool __erase(VirtualAddress addr, size_t size, ReleaseFunc release, SplitFunc split) {
        uint32_t upperSum = 0;
        if (addr >= TopAddr || __builtin_add_overflow(addr.addr, static_cast<uint32_t>(size), &upperSum) ||
            upperSum > Top) {
            return false;
        }
        auto upper = paging::VirtualAddress{upperSum};

        auto left = __locate(addr);
        if (!left->contains(addr)) {
            left = std::next(left);
            if (left == __addrs.end()) {
                return true;
            }
        }

        auto right = __locate(upper);
        if (right->__begin >= upper) {
            if (right == __addrs.begin()) {
                return true;
            }
            right = std::prev(right);
        }

        auto lower = addr;

        if (left > right) {
            return true;
        } else if (left == right) {
            // 仅影响这一页
            if (left->__begin >= lower) {
                if (left->__end <= upper) {
                    release(left->__begin, left->__end);
                    __addrs.erase(left);
                    return true;
                } else {
                    release(left->__begin, upper);
                    left->__begin = upper;
                    return true;
                }
            } else if (left->__end <= upper) {
                release(lower, left->__end);
                left->__end = lower;
                return true;
            } else {
                release(lower, upper);
                RangeEntry entry{
                    left->__begin,
                    lower,
                };
                split(entry, *left);
                left->__begin = upper;
                __addrs.insert(left, entry);
                return true;
            }
        } else {
            auto first_to_remove = left;
            auto last_to_remove = right;
            if (left->__begin < lower) {
                release(lower, left->__end);
                left->__end = lower;
                first_to_remove = std::next(first_to_remove);
            }

            if (right->__end > upper) {
                release(right->__begin, upper);
                right->__begin = upper;
                last_to_remove = std::prev(right);
            }

            if (first_to_remove <= last_to_remove) {
                last_to_remove = std::next(last_to_remove);
                for (auto it = first_to_remove; it != last_to_remove; it++) {
                    release(it->__begin, it->__end);
                }
                __addrs.erase(first_to_remove, last_to_remove);
            }

            return true;
        }
    }
};

}  // namespace nyan::paging
