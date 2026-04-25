#include "load.hpp"

#include "frame.hpp"
#include "physicalFrame.hpp"
#include "pool.hpp"
#include "slab.hpp"

namespace nyan::allocator {

alignas(PoolManager) char __poolStorage[sizeof(PoolManager)];
alignas(FrameManager) char __frameStorage[sizeof(FrameManager)];
alignas(SlabManager) char __slabStorage[sizeof(SlabManager)];
alignas(PhysicalFrameManager) char __physicalFrameStorage[sizeof(PhysicalFrameManager)];

PoolManager* poolManager;
FrameManager* frameManager;
SlabManager* slabManager;
PhysicalFrameManager* physicalFrameManager;

void load(uint32_t upper) {
    poolManager = new (__poolStorage) PoolManager(upper - poolBase);
    frameManager = new (__frameStorage) FrameManager;
    slabManager = new (__slabStorage) SlabManager;
    physicalFrameManager = new (__physicalFrameStorage) PhysicalFrameManager(upper - poolBase);
}

}  // namespace nyan::allocator
