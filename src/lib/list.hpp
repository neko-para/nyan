#pragma once

#include "list/container.hpp"

namespace nyan::lib {

template <typename... Tags>
using ListNodes = __list::ListNodes<Tags...>;

template <typename Tag, bool WithTail>
using List = __list::List<Tag, WithTail>;

}  // namespace nyan::lib
