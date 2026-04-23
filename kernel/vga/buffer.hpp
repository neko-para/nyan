#pragma once

#include <sys/types.h>

#include "entry.hpp"

namespace nyan::vga {

constexpr size_t width = 80;
constexpr size_t height = 25;

extern "C" volatile Entry buffer[width * height] asm("vga_buffer");

}  // namespace nyan::vga
