#pragma once

#include <vector>

#include "../lib/shared.hpp"
#include "forward.hpp"
#include "mount.hpp"

namespace nyan::fs {

extern std::vector<lib::Ref<MountEntry>>* mountPoints;
extern RamFS* ramFS;

void load();

inline auto rootEntry() {
    return (*mountPoints)[0];
}

}  // namespace nyan::fs
