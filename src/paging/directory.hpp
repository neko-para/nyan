#pragma once

#include <stdint.h>

#include "../arch/io.hpp"
#include "../lib/function.hpp"
#include "convert.hpp"
#include "table.hpp"

namespace nyan::paging {

struct MapperGuard;

struct alignas(4096) Directory {
    uint32_t data[1024];

    // TODO: 清理下这三个函数
    void load() const noexcept { asm volatile("movl %0, %%cr3" ::"r"(this) : "memory"); }
    PhysicalAddress cr3() const noexcept { return VirtualAddress{reinterpret_cast<uint32_t>(this)}.kernelToPhysical(); }
    static Directory* fromCr3(uint32_t addr) noexcept { return physicalToVirtual(reinterpret_cast<Directory*>(addr)); }

    void clear() noexcept { std::fill_n(data, 1024, 0); }

    void set(PhysicalAddress table, uint16_t location, uint16_t attr) noexcept { data[location] = table.addr | attr; }
    PhysicalAddress at(uint16_t location) const noexcept { return {data[location] & (~0xFFF)}; }
    bool present(uint16_t location) const noexcept { return data[location] & PDE_Present; }

    // 仅用于内核. 用户态不是线性映射
    void map(PairedAddress addrs, uint16_t attr) noexcept {
        auto table = at(addrs.vAddr.addr >> 22);
        if (!table) {
            arch::kfatal("table not exists");
        }
        table.kernelToVirtual().as<Table>()->map(addrs, attr);
    }
    bool unmap(VirtualAddress virtualAddr, PhysicalAddress& physicalAddr) noexcept {
        auto table = at(virtualAddr.addr >> 22);
        if (!table) {
            arch::kfatal("table not exists");
        }
        return table.kernelToVirtual().as<Table>()->unmap(virtualAddr, physicalAddr);
    }

    MapperGuard fork() const noexcept;
    void ensure(uint16_t location, uint16_t attr) noexcept;
    void with(uint16_t location, lib::function<void(Table*)> func) const noexcept;
};

}  // namespace nyan::paging
