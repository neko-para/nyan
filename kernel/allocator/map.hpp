#pragma once

#include <sys/types.h>

namespace nyan::allocator {

constexpr uint32_t __pool_base = 0x800000;

constexpr uint32_t __frame_base = 0xC0800000;
constexpr uint32_t __frame_top = 0xD0000000;

constexpr uint32_t __large_frame_base = 0xD0000000;
constexpr uint32_t __large_frame_top = 0xFFFFF000;

}  // namespace nyan::allocator
