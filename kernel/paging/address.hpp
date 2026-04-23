#pragma once

#include <sys/types.h>
#include <compare>

#include "../arch/instr.hpp"

namespace nyan::paging {

struct PhysicalAddress;
struct VirtualAddress;

template <typename Addr>
struct BaseAddress {
    uint32_t addr;

    constexpr explicit BaseAddress(uint32_t addr = 0) : addr(addr) {}
    constexpr explicit BaseAddress(int addr) : addr(addr) {}
    explicit BaseAddress(const void* ptr) : addr(reinterpret_cast<uint32_t>(ptr)) {}

    constexpr explicit operator bool() const noexcept { return addr; }
    constexpr explicit operator uint32_t() const noexcept = delete;

    constexpr auto operator<=>(const BaseAddress<Addr>& a) const noexcept = default;

    constexpr Addr alignDown() const noexcept { return Addr{addr & (~0xFFF)}; }
    constexpr Addr alignUp() const noexcept { return Addr{(addr + 0xFFF) & (~0xFFF)}; }

    constexpr Addr thisPage() const noexcept { return alignDown(); }
    constexpr Addr prevPage() const noexcept { return Addr{(addr & (~0xFFF)) - 0x1000}; }
    constexpr Addr nextPage() const noexcept { return Addr{(addr & (~0xFFF)) + 0x1000}; }

    constexpr uint32_t pageOffset() const noexcept { return addr & 0xFFF; }

    constexpr Addr operator+(int32_t offset) const noexcept { return Addr{static_cast<uint32_t>(addr + offset)}; }
    constexpr Addr operator-(int32_t offset) const noexcept { return Addr{static_cast<uint32_t>(addr - offset)}; }
    constexpr int64_t operator-(const Addr& a) const noexcept {
        return static_cast<int64_t>(addr) - static_cast<int64_t>(a.addr);
    }
};

struct PhysicalAddress : public BaseAddress<PhysicalAddress> {
    using BaseAddress::BaseAddress;

    VirtualAddress kernelToVirtual() const noexcept;

    void setCr3() const noexcept { arch::setCr3(addr); }

    template <typename T>
    T* unsafeAs() const noexcept {
        return reinterpret_cast<T*>(addr);
    }
};

struct VirtualAddress : public BaseAddress<VirtualAddress> {
    using BaseAddress::BaseAddress;

    PhysicalAddress kernelToPhysical() const noexcept;

    void invlpg() const noexcept { arch::invlpg(addr); }
    constexpr uint16_t tableLoc() const noexcept { return addr >> 22; }

    template <typename T>
    T* as() const noexcept {
        return reinterpret_cast<T*>(addr);
    }
};

inline VirtualAddress PhysicalAddress::kernelToVirtual() const noexcept {
    return VirtualAddress{addr + 0xC0000000};
}

inline PhysicalAddress VirtualAddress::kernelToPhysical() const noexcept {
    return PhysicalAddress{addr - 0xC0000000};
}

}  // namespace nyan::paging

constexpr nyan::paging::PhysicalAddress operator""_pa(uint64_t addr) {
    return nyan::paging::PhysicalAddress{static_cast<uint32_t>(addr)};
}

constexpr nyan::paging::VirtualAddress operator""_va(uint64_t addr) {
    return nyan::paging::VirtualAddress{static_cast<uint32_t>(addr)};
}
