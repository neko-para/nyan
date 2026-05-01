#pragma once

#include "../lib/shared.hpp"
#include "forward.hpp"

namespace nyan::fs {

struct SuperBlock : public lib::Shared {
    FileSystem* __fs{};
    MountEntry* __entry{};
};

}  // namespace nyan::fs
