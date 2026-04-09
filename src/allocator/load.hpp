#pragma once

#include <stdint.h>

namespace nyan::allocator {

struct PoolManager;
struct FrameManager;
struct SlabManager;

extern PoolManager* poolManager;
extern FrameManager* frameManager;
extern SlabManager* slabManager;

void load(uint32_t upper);

}  // namespace nyan::allocator
