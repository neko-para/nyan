#pragma once

#include <sys/types.h>

namespace nyan::allocator {

constexpr uint32_t frameBase = 0xC0800000;
constexpr uint32_t frameTop = 0xD0000000;

constexpr uint32_t largeFrameBase = 0xD0000000;
constexpr uint32_t largeFrameTop = 0xFFFFF000;

}  // namespace nyan::allocator
