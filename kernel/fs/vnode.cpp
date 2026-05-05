#include "vnode.hpp"

#include "vnode_file.hpp"

namespace nyan::fs {

lib::Ref<VNodeFileObj> VNode::open(lib::Ref<VNode> self, uint32_t mode) noexcept {
    return lib::makeRef<VNodeFileObj>(self, mode);
}

}  // namespace nyan::fs