#pragma once

#include <stdint.h>

#include "../arch/io.hpp"
#include "convert.hpp"
#include "table.hpp"

namespace nyan::paging {

struct alignas(4096) Directory {
    uint32_t data[1024];

    void load() const noexcept { asm volatile("movl %0, %%cr3" ::"r"(this) : "memory"); }
    void clear() noexcept { std::fill_n(data, 1024, 0); }
    void set(Table* table, uint16_t location, uint16_t attr) noexcept {
        data[location] = reinterpret_cast<uint32_t>(table) | attr;
    }
    Table* at(uint16_t location) const noexcept { return reinterpret_cast<Table*>(data[location] & (~0xFFF)); }
    uint32_t cr3() const noexcept { return virtualToPhysical(reinterpret_cast<uint32_t>(this)); }
    static Directory* fromCr3(uint32_t addr) noexcept { return physicalToVirtual(reinterpret_cast<Directory*>(addr)); }

    void map(uint32_t physicalAddr, uint32_t virtualAddr, uint16_t attr) noexcept {
        auto table = at(virtualAddr >> 22);
        if (!table) {
            arch::kfatal("table not exists");
        }
        physicalToVirtual(table)->map(physicalAddr, virtualAddr, attr);
    }
    bool unmap(uint32_t virtualAddr, uint32_t& physicalAddr) noexcept {
        auto table = at(virtualAddr >> 22);
        if (!table) {
            arch::kfatal("table not exists");
        }
        return physicalToVirtual(table)->unmap(virtualAddr, physicalAddr);
    }

    Directory* fork() const noexcept {
        Directory* other = allocator::frameAllocAs<Directory>();
        std::fill_n(other->data, 768, 0);
        std::copy_n(data + 768, 256, other->data + 768);
        return other;
    }
};

}  // namespace nyan::paging
