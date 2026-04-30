#pragma once

#include <string_view>
#include <vector>

#include "../lib/shared.hpp"
#include "forward.hpp"

namespace nyan::fs {

extern std::vector<MountEntry>* mountPoints;
extern RamFS* ramFS;
extern SuperBlock* rootSuperBlock;

void load();
lib::Ref<VNode> resolve(std::string_view path);

}  // namespace nyan::fs
