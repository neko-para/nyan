#pragma once

#include "../containers.hpp"
#include "concepts.hpp"

namespace nyan::lib::__format {

// no `:` support
// {<here>}
using parse_context = std::string_view;

template <output_iterator Iter>
struct format_context {
    using iterator = Iter;

    Iter iter;
};

}  // namespace nyan::lib::__format
