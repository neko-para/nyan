#pragma once

#include <concepts>
#include <tuple>

namespace nyan::lib {

template <typename T, typename... Ts>
concept same_as_any = (std::same_as<T, Ts> || ...);

template <typename T, typename Tuple>
constexpr bool same_as_any_in_tuple_v = false;
template <typename T, typename... Ts>
constexpr bool same_as_any_in_tuple_v<T, std::tuple<Ts...>> = same_as_any<T, Ts...>;
template <typename T, typename Tuple>
concept same_as_any_in_tuple = same_as_any_in_tuple_v<T, Tuple>;

}  // namespace nyan::lib
