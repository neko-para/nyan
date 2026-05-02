#include "frame.hpp"
#include "large_frame.hpp"
#include "mod.hpp"
#include "physical_frame.hpp"
#include "pool.hpp"
#include "slab.hpp"

namespace nyan::allocator {

alignas(PoolManager) static char __pool_storage[sizeof(PoolManager)];
alignas(FrameManager) static char __frame_storage[sizeof(FrameManager)];
alignas(SlabManager) static char __slab_storage[sizeof(SlabManager)];
alignas(PhysicalFrameManager) static char __physical_frame_storage[sizeof(PhysicalFrameManager)];

PoolManager* __pool_manager;
FrameManager* __frame_manager;
SlabManager* __slab_manager;
PhysicalFrameManager* __physical_frame_manager;
LargeFrameManager* __large_frame_manager;

void load(uint32_t upper) noexcept {
    __pool_manager = new (__pool_storage) PoolManager(upper - __pool_base);
    __frame_manager = new (__frame_storage) FrameManager;
    __slab_manager = new (__slab_storage) SlabManager;
    __physical_frame_manager = new (__physical_frame_storage) PhysicalFrameManager(upper - __pool_base);
    __large_frame_manager = new LargeFrameManager;
}

}  // namespace nyan::allocator
