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

MapperGuard UserDirectory::alloc(VirtualAddress addr, bool writable) {
    auto tableLocation = addr.tableLoc();
    ensure(tableLocation, paging::PDE_Present | paging::PDE_ReadWrite | paging::PDE_User);
    auto physicalAddr = with(tableLocation, [&](paging::Table* table) {
        return table->alloc(addr, paging::PTE_Present | (writable ? paging::PTE_ReadWrite : 0) | paging::PTE_User);
    });

    paging::MapperGuard mapper(physicalAddr);
    auto frame = mapper.as<uint8_t>();
    std::fill_n(frame, 0x1000, 0);
    return mapper;
}

}  // namespace nyan::paging
