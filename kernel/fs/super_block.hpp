#pragma once

#include "../lib/shared.hpp"
#include "vnode.hpp"

namespace nyan::fs {

struct SuperBlock {
    FileSystem* __fs;
    lib::Ref<VNode> __root;
};

}  // namespace nyan::fs
