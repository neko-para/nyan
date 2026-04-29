#pragma once

#include "forward.hpp"

namespace nyan::fs {

extern RamFS* ramFS;
extern SuperBlock* rootSuperBlock;

void load();

}  // namespace nyan::fs
