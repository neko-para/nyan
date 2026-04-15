#pragma once

#include <stdint.h>

#include "../arch/instr.hpp"

namespace nyan::paging {

struct PhysicalAddress;
struct VirtualAddress;

struct PhysicalAddress {
    uint32_t addr;

    explicit PhysicalAddress(uint32_t addr = 0) : addr(addr) {}
    explicit PhysicalAddress(int addr) : addr(addr) {}
    explicit PhysicalAddress(const void* ptr) : addr(reinterpret_cast<uint32_t>(ptr)) {}

    operator bool() const noexcept { return addr; }
    operator uint32_t() const noexcept = delete;

    VirtualAddress kernelToVirtual() const noexcept;

    void setCr3() const noexcept { arch::setCr3(addr); }

    template <typename T>
    T* unsafeAs() const noexcept {
        return reinterpret_cast<T*>(addr);
    }
};

struct VirtualAddress {
    uint32_t addr;

    explicit VirtualAddress(uint32_t addr = 0) : addr(addr) {}
    explicit VirtualAddress(int addr) : addr(addr) {}
    explicit VirtualAddress(const void* ptr) : addr(reinterpret_cast<uint32_t>(ptr)) {}

    operator bool() const noexcept { return addr; }
    operator uint32_t() const noexcept = delete;

    PhysicalAddress kernelToPhysical() const noexcept;

    void invlpg() const noexcept { arch::invlpg(addr); }
    uint16_t tableLoc() const noexcept { return addr >> 22; }

    template <typename T>
    T* as() const noexcept {
        return reinterpret_cast<T*>(addr);
    }
};

struct PairedAddress {
    PhysicalAddress pAddr;
    VirtualAddress vAddr;
};

inline VirtualAddress PhysicalAddress::kernelToVirtual() const noexcept {
    return VirtualAddress{addr + 0xC0000000};
}

inline PhysicalAddress VirtualAddress::kernelToPhysical() const noexcept {
    return PhysicalAddress{addr - 0xC0000000};
}

}  // namespace nyan::paging
