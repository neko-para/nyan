#pragma once

#include <stdint.h>

namespace nyan::paging {

struct alignas(4096) Table {
    uint32_t data[1024];
};

}  // namespace nyan::paging
