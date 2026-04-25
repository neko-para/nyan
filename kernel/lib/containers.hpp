#pragma once

#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace nyan::lib {

using string = std::basic_string<char>;
using string_view = std::string_view;
template <typename T>
using vector = std::vector<T>;
template <typename T>
using deque = std::deque<T>;
template <typename T>
using unique_ptr = std::unique_ptr<T>;

}  // namespace nyan::lib
