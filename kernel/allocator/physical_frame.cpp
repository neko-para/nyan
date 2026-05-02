#include "physical_frame.hpp"

namespace nyan::allocator {

alignas(PhysicalFrame) static char __physical_frame_info_storage[sizeof(PhysicalFrame) << 20];

PhysicalFrameManager::PhysicalFrameManager(size_t size) noexcept {
    __info = new (__physical_frame_info_storage) PhysicalFrame[size >> 12];
    std::fill_n(__info, size >> 12, PhysicalFrame{});
}

}  // namespace nyan::allocator