#pragma once

#include <concepts>

#include "../concepts.hpp"
#include "../containers.hpp"

namespace nyan::lib::__format {

template <typename Iter>
concept output_iterator = std::output_iterator<Iter, const char&>;

template <typename T>
constexpr bool is_char_array_v = false;
template <size_t N>
constexpr bool is_char_array_v<char[N]> = true;
template <typename T>
concept is_char_array = is_char_array_v<T>;
template <typename T>
constexpr size_t char_array_size = 0;
template <size_t N>
constexpr size_t char_array_size<char[N]> = N;

template <typename T>
concept target_string = same_as_any<T, const char*, char*, string_view, string> || is_char_array<T>;

template <typename T>
concept target_integral = std::integral<T> && !same_as_any<T, bool, char>;

template <typename T>
concept target_char = std::same_as<T, char>;

template <typename T>
concept target_bool = std::same_as<T, bool>;

template <typename T>
concept target_pointer = (std::is_pointer_v<T> && !same_as_any<T, const char*, char*>) || std::same_as<T, nullptr_t>;

template <typename T>
concept target_numeric = target_integral<T> || target_char<T> || target_bool<T>;

template <typename T>
concept known_target = target_string<T> || target_integral<T> || target_char<T> || target_bool<T> || target_pointer<T>;

}  // namespace nyan::lib::__format
