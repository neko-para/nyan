#pragma once

#include <stdint.h>
#include <sys/types.h>

namespace nyan::allocator {

struct PoolManager;
struct FrameManager;
struct SlabManager;
struct PhysicalFrameManager;
struct LargeFrameManager;

extern PoolManager* poolManager;
extern FrameManager* frameManager;
extern SlabManager* slabManager;
extern PhysicalFrameManager* physicalFrameManager;
extern LargeFrameManager* largeFrameManager;

void load(uint32_t upper);

}  // namespace nyan::allocator
