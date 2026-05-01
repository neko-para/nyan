#pragma once

#include "../lib/shared.hpp"
#include "forward.hpp"

namespace nyan::fs {

struct MountEntry : lib::Shared {
    lib::Ref<DEntry> __mount_point;
    lib::Ref<SuperBlock> __super_block;
    lib::Ref<VNode> __root_node;
};

}  // namespace nyan::fs
