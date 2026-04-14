#pragma once

#include <stdint.h>

namespace nyan::allocator {

struct PoolManager;
struct FrameManager;
struct SlabManager;
struct PhysicalFrameManager;

extern PoolManager* poolManager;
extern FrameManager* frameManager;
extern SlabManager* slabManager;
extern PhysicalFrameManager* physicalFrameManager;

void load(uint32_t upper);

}  // namespace nyan::allocator
