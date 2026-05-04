#include "forward.hpp"
#include "frame.hpp"
#include "large_frame.hpp"
#include "mod.hpp"
#include "physical_frame.hpp"
#include "pool.hpp"
#include "slab.hpp"

namespace nyan::allocator {

lib::Lazy<PoolManager> __pool_manager;
lib::Lazy<FrameManager> __frame_manager;
lib::Lazy<SlabManager> __slab_manager;
lib::Lazy<PhysicalFrameManager> __physical_frame_manager;
lib::Lazy<LargeFrameManager> __large_frame_manager;

void load(uint32_t upper) noexcept {
    __pool_manager.construct(upper - __pool_base);
    __frame_manager.construct();
    __slab_manager.construct();
    __physical_frame_manager.construct(upper - __pool_base);
    __large_frame_manager.construct();
}

}  // namespace nyan::allocator
