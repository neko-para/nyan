#include "load.hpp"

#include "frame.hpp"
#include "physicalFrame.hpp"
#include "pool.hpp"
#include "slab.hpp"

namespace nyan::allocator {

PoolManager* poolManager;
FrameManager* frameManager;
SlabManager* slabManager;
PhysicalFrameManager* physicalFrameManager;

void load(uint32_t upper) {
    poolManager = new PoolManager(upper - poolBase);
    frameManager = new FrameManager;
    slabManager = new SlabManager;
    physicalFrameManager = new PhysicalFrameManager(upper - poolBase);
}

}  // namespace nyan::allocator
