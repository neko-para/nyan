#pragma once

#include <stddef.h>
#include <stdint.h>
#include <algorithm>

#include "entry.hpp"

namespace nyan::paging {

struct alignas(4096) Table {
    uint32_t data[1024];

    void clear() noexcept { std::fill_n(data, 1024, 0); }
    void fillFlat(uint32_t base, uint16_t attr) noexcept {
        for (size_t i = 0; i < 1024; i++) {
            data[i] = (base + (i << 12)) | attr;
        }
    }

    void map(uint32_t physicalAddr, uint32_t virtualAddr, uint16_t attr) noexcept {
        auto location = (virtualAddr >> 12) & 0x3FF;
        data[location] = (physicalAddr & (~0x3FF)) | attr;
    }
    bool unmap(uint32_t virtualAddr, uint32_t& physicalAddr) noexcept {
        auto location = (virtualAddr >> 12) & 0x3FF;
        if (data[location] & PTE_Present) {
            physicalAddr = data[location] & (~0x3FF);
            data[location] = 0;
            return true;
        } else {
            return false;
        }
    }
};

}  // namespace nyan::paging
