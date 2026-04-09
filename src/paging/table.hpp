#pragma once

#include <stddef.h>
#include <stdint.h>

namespace nyan::paging {

struct alignas(4096) Table {
    uint32_t data[1024];

    void fillFlat(uint32_t base, uint16_t attr) noexcept {
        for (size_t i = 0; i < 1024; i++) {
            data[i] = (base + (i << 12)) | attr;
        }
    }
};

}  // namespace nyan::paging
