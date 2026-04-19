#pragma once

#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "../allocator/allocator.hpp"

namespace nyan::lib {

using string = std::basic_string<char, std::char_traits<char>, allocator::SlabAllocator<char>>;
using string_view = std::basic_string_view<char, std::char_traits<char>>;
template <typename T>
using vector = std::vector<T, allocator::SlabAllocator<T>>;
template <typename T>
using deque = std::deque<T, allocator::SlabAllocator<T>>;
template <typename T>
using unique_ptr = std::unique_ptr<T, allocator::SlabAllocatorDeletor<T>>;

}  // namespace nyan::lib
