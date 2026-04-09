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
        return physicalToVirtual(table)->unmap(physicalAddr, virtualAddr);
    }
};

}  // namespace nyan::paging
