#pragma once

#include "../lib/shared.hpp"
#include "vnode.hpp"

namespace nyan::fs {

struct MountEntry {
    lib::Ref<VNode> __mount_point;
    SuperBlock* __super_block;
};

}  // namespace nyan::fs
