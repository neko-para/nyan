#include "kernel.hpp"

namespace nyan::paging {

Directory kernelPageDirectory;
Table kernelPageTable[256];

// RUN IN LOWER
extern "C" void preparePaging() {
    auto cr3Addr = paging::VirtualAddress(&kernelPageDirectory).kernelToPhysical();
    auto& phyDirectory = *cr3Addr.unsafeAs<Directory>();

    auto tableAddr = paging::VirtualAddress{kernelPageTable}.kernelToPhysical();
    auto& phyTable = *tableAddr.unsafeAs<Table[256]>();

    phyDirectory.clear();
    for (auto& table : phyTable) {
        table.clear();
    }

    for (size_t i = 0; i < 256; i++) {
        PhysicalAddress addr = {
            tableAddr.addr + i * sizeof(Table),
        };
        phyDirectory.set(addr, i, PDE_Present | PDE_ReadWrite);
        phyDirectory.set(addr, i | (3 << 8), PDE_Present | PDE_ReadWrite);
    }
    phyTable[0].fillFlat(0, paging::PTE_Present | paging::PTE_ReadWrite);
    phyTable[0].map({.pAddr = {0xB8000}, .vAddr = {0xC03FF000}}, paging::PTE_Present | paging::PTE_ReadWrite);

    cr3Addr.setCr3();

    asm volatile(
        "movl %%cr0, %%eax;"
        "orl $0x80010000, %%eax;"
        "movl %%eax, %%cr0" ::
            : "eax", "memory");
}

void clearIdentityPaging() {
    for (size_t i = 0; i < 256; i++) {
        kernelPageDirectory.set({0}, i, 0);
    }

    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;" ::
            : "eax", "memory");
}

MapperGuard Directory::fork() const noexcept {
    auto physicalAddr = allocator::physicalFrameAlloc();
    MapperGuard mapper(physicalAddr);
    auto other = mapper.as<Directory>();
    std::fill_n(other->data, 768, 0);
    std::copy_n(data + 768, 256, other->data + 768);
    return mapper;
}

void Directory::ensure(uint16_t location, uint16_t attr) noexcept {
    if (!present(location)) {
        paging::MapperGuard mapper(allocator::physicalFrameAlloc());
        mapper.as<paging::Table>()->clear();
        set(mapper.paddr, location, attr);
    }
}

void Directory::with(uint16_t location, lib::function<void(Table*)> func) const noexcept {
    MapperGuard mapper(at(location));
    func(mapper.as<Table>());
}

}  // namespace nyan::paging
