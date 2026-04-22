#pragma once

#include "lib/list/container.hpp"

namespace nyan {

struct Item;

struct Tag {
    using type = Item;
};

struct Item : public lib::__list::ListNodes<Tag> {
    int value;
};

}  // namespace nyan
