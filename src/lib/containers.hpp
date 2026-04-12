#pragma once

#include <string>
#include <vector>

#include "../allocator/allocator.hpp"

namespace nyan::lib {

using string = std::basic_string<char, std::char_traits<char>, allocator::SlabAllocator<char>>;
using string_view = std::basic_string_view<char, std::char_traits<char>>;
template <typename T>
using vector = std::vector<T, allocator::SlabAllocator<T>>;

}  // namespace nyan::lib
