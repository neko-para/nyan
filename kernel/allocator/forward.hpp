#pragma once

namespace nyan::allocator {

struct PoolManager;
struct FrameManager;
struct SlabManager;
struct PhysicalFrameManager;
struct LargeFrameManager;

extern PoolManager* __pool_manager;
extern FrameManager* __frame_manager;
extern SlabManager* __slab_manager;
extern PhysicalFrameManager* __physical_frame_manager;
extern LargeFrameManager* __large_frame_manager;

}  // namespace nyan::allocator