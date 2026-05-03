#pragma once

#include <stdint.h>

#include "../interrupt/forward.hpp"

namespace nyan::keyboard {

void load() noexcept;
bool push(uint8_t dat, interrupt::SyscallFrame* frame) noexcept;

}  // namespace nyan::keyboard