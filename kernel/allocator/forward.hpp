#pragma once

#include "../lib/lazy.hpp"

namespace nyan::allocator {

struct PoolManager;
struct FrameManager;
struct SlabManager;
struct PhysicalFrameManager;
struct LargeFrameManager;

extern lib::Lazy<PoolManager> __pool_manager;
extern lib::Lazy<FrameManager> __frame_manager;
extern lib::Lazy<SlabManager> __slab_manager;
extern lib::Lazy<PhysicalFrameManager> __physical_frame_manager;
extern lib::Lazy<LargeFrameManager> __large_frame_manager;

}  // namespace nyan::allocator