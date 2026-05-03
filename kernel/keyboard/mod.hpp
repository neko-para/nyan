#pragma once

#include <stdint.h>

#include "../interrupt/forward.hpp"

namespace nyan::keyboard {

void load() noexcept;
bool push(uint8_t dat) noexcept;

}  // namespace nyan::keyboard