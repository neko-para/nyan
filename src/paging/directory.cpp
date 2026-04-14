#include "directory.hpp"

namespace nyan::paging {

UserDirectory UserDirectory::fork(const KernelDirectory& directory) noexcept {
    auto physicalAddr = allocator::physicalFrameAlloc();
    UserDirectory userDir{MapperGuard{physicalAddr}};
    auto data = userDir->data;
    std::fill_n(data, 768, 0);
    std::copy_n(directory.data + 768, 256, data + 768);
    return userDir;
}

}  // namespace nyan::paging
