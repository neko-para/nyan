#include "directory.hpp"

#include "../allocator/mod.hpp"

namespace nyan::paging {

MapperGuard::MapperGuard(PhysicalAddress addr) noexcept : paddr(addr) {
    vaddr = allocator::virtualFrameAlloc();
    kernelPageDirectory.map(vaddr, paddr, PTE_Present | PTE_ReadWrite);
    vaddr.invlpg();
}

MapperGuard::~MapperGuard() {
    if (vaddr) {
        PhysicalAddress _;
        kernelPageDirectory.unmap({vaddr}, _);
        vaddr.invlpg();
        allocator::virtualFrameFree(vaddr);
    }
}

UserDirectory UserDirectory::fork(const KernelDirectory& directory) noexcept {
    auto physicalAddr = allocator::physicalFrameAlloc();
    UserDirectory userDir{MapperGuard{physicalAddr}};
    auto data = userDir->data;
    std::copy_n(directory.data + 768, 256, data + 768);
    std::fill_n(data, 768, 0);
    return userDir;
}

UserDirectory UserDirectory::forkCOW(const UserDirectory& oldDir) noexcept {
    auto physicalAddr = allocator::physicalFrameAlloc();
    UserDirectory newDir{MapperGuard{physicalAddr}};
    auto data = newDir->data;
    std::copy_n(oldDir.data()->data + 768, 256, data + 768);
    std::fill_n(data, 768, 0);
    for (uint16_t i = 0; i < 768; i++) {
        if (!oldDir->isPresent(i)) {
            continue;
        }
        oldDir.with(i, [&](Table* oldTable) {
            newDir.ensure(i, oldDir.data()->attr(i));
            newDir.with(i, [&](Table* newTable) {
                for (uint16_t j = 0; j < 1024; j++) {
                    if (!oldTable->isPresent(j)) {
                        continue;
                    }
                    auto addr = oldTable->at(j);
                    auto attr = oldTable->attr(j);
                    if (attr & PTE_ReadWrite) {
                        attr &= ~PTE_ReadWrite;
                        attr |= PTE_COW;
                    }
                    oldTable->data[j] = addr.addr | attr;
                    newTable->data[j] = addr.addr | attr;
                    allocator::physicalFrameRetain(addr);
                }
            });
        });
    }
    return newDir;
}

void UserDirectory::ensure(uint16_t location, uint16_t attr) noexcept {
    if (!data()->isPresent(location)) {
        paging::MapperGuard mapper(allocator::physicalFrameAlloc());
        mapper.as<paging::Table>()->clear();
        data()->set(mapper.paddr, location, attr);
    }
}

void UserDirectory::freePage(VirtualAddress addr) noexcept {
    auto loc = addr.tableLoc();
    if (data()->isPresent(loc)) {
        with(loc, [addr](Table* table) { table->freePage(addr); });
    }
}

void UserDirectory::freePageTables() noexcept {
    for (uint16_t i = 0; i < 768; i++) {
        if (data()->isPresent(i)) {
            with(i, [i](Table* table) { table->freeDangling(i); });
            allocator::physicalFrameRelease(data()->at(i));
            data()->data[i] = 0;
        }
    }
}

MapperGuard UserDirectory::alloc(VirtualAddress addr, bool writable) noexcept {
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

bool UserDirectory::handleCOW(VirtualAddress addr) noexcept {
    auto tableLocation = addr.tableLoc();
    if (!data()->isPresent(tableLocation)) {
        return false;
    }

    return with(tableLocation, [&](Table* table) {
        auto offset = addr.pageTableOffset();
        if (!table->isPresent(offset)) {
            return false;
        }
        auto attr = table->attr(offset);
        if (!(attr & PTE_COW)) {
            return false;
        }
        auto oldAddr = table->at(offset);

        if (allocator::physicalFrameGetRef(oldAddr) == 1) {
            attr |= PTE_ReadWrite;
            attr &= ~PTE_COW;
            table->data[offset] = oldAddr.addr | attr;
            addr.invlpg();
            return true;
        }

        auto newAddr = allocator::physicalFrameAlloc();

        MapperGuard oldMapper(oldAddr);
        MapperGuard newMapper(newAddr);
        std::copy_n(oldMapper.as<uint8_t>(), 4096, newMapper.as<uint8_t>());

        allocator::physicalFrameRelease(oldAddr);

        attr |= PTE_ReadWrite;
        attr &= ~PTE_COW;
        table->data[offset] = newAddr.addr | attr;
        addr.invlpg();
        return true;
    });
}

}  // namespace nyan::paging
