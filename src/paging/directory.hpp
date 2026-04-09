#pragma once

#include <stdint.h>

namespace nyan::paging {

struct Table;

struct alignas(4096) Directory {
    uint32_t data[1024];

    void load() const noexcept { asm volatile("movl %0, %%cr3" ::"r"(this) : "memory"); }
    void set(Table* table, uint16_t location, uint16_t attr) noexcept {
        data[location] = reinterpret_cast<uint32_t>(table) | attr;
    }
};

}  // namespace nyan::paging
