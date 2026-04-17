#pragma once

#include <sys/types.h>

#include "../arch/io.hpp"
#include "table.hpp"

namespace nyan::paging {

struct MapperGuard;
struct KernelDirectory;
struct UserDirectory;

extern KernelDirectory kernelPageDirectory;

struct alignas(4096) DirectoryData {
    uint32_t data[1024];

    void clear() noexcept { std::fill_n(data, 1024, 0); }

    void set(PhysicalAddress table, uint16_t location, uint16_t attr) noexcept { data[location] = table.addr | attr; }
    PhysicalAddress at(uint16_t location) const noexcept { return PhysicalAddress{data[location] & (~0xFFF)}; }
    bool isPresent(uint16_t location) const noexcept { return data[location] & PDE_Present; }
};

struct KernelDirectory : public DirectoryData {
    void map(PairedAddress addrs, uint16_t attr) const noexcept {
        auto table = at(addrs.vAddr.addr >> 22);
        if (!table) {
            arch::kfatal("table not exists");
        }
        table.kernelToVirtual().as<Table>()->map(addrs, attr);
    }

    bool unmap(VirtualAddress virtualAddr, PhysicalAddress& physicalAddr) const noexcept {
        auto table = at(virtualAddr.addr >> 22);
        if (!table) {
            arch::kfatal("table not exists");
        }
        return table.kernelToVirtual().as<Table>()->unmap(virtualAddr, physicalAddr);
    }

    PhysicalAddress cr3() const noexcept { return paging::VirtualAddress{this}.kernelToPhysical(); }
};

struct MapperGuard {
    MapperGuard(PhysicalAddress addr) noexcept : paddr(addr) {
        vaddr = allocator::virtualFrameAlloc();
        kernelPageDirectory.map(
            {
                .pAddr = paddr,
                .vAddr = vaddr,
            },
            PTE_Present | PTE_ReadWrite);
        vaddr.invlpg();
    }
    MapperGuard(const MapperGuard&) = delete;
    MapperGuard(MapperGuard&& mapper) noexcept : paddr(mapper.paddr), vaddr(mapper.vaddr.addr) {
        mapper.paddr = PhysicalAddress{0};
        mapper.vaddr = VirtualAddress{0};
    }
    ~MapperGuard() noexcept {
        if (vaddr) {
            PhysicalAddress _;
            kernelPageDirectory.unmap({vaddr}, _);
            vaddr.invlpg();
            allocator::virtualFrameFree(vaddr);
        }
    }
    MapperGuard& operator=(const MapperGuard&) = delete;
    MapperGuard& operator=(MapperGuard&& mapper) noexcept {
        if (this == &mapper) {
            return *this;
        }
        this->~MapperGuard();
        paddr = mapper.paddr;
        vaddr = mapper.vaddr;
        mapper.paddr = PhysicalAddress{0};
        mapper.vaddr = VirtualAddress{0};
        return *this;
    }

    template <typename T>
    T* as() const noexcept {
        return vaddr.as<T>();
    }

    PhysicalAddress paddr;
    VirtualAddress vaddr;
};

struct UserDirectory {
    MapperGuard mapper;

    DirectoryData* data() const noexcept { return mapper.as<DirectoryData>(); }
    DirectoryData* operator->() const noexcept { return data(); }

    static UserDirectory from(PhysicalAddress addr) noexcept { return {{addr}}; }
    static UserDirectory fork(const KernelDirectory& directory) noexcept;

    void ensure(uint16_t location, uint16_t attr) noexcept {
        if (!data()->isPresent(location)) {
            paging::MapperGuard mapper(allocator::physicalFrameAlloc());
            mapper.as<paging::Table>()->clear();
            data()->set(mapper.paddr, location, attr);
        }
    }

    template <typename Func>
    auto with(uint16_t location, Func func) const noexcept {
        MapperGuard mapper(data()->at(location));
        return func(mapper.as<Table>());
    }

    void free() {
        for (size_t i = 0; i < 768; i++) {
            if (!data()->isPresent(i)) {
                continue;
            }
            with(i, [](Table* table) { table->free(); });
            allocator::physicalFrameRelease(data()->at(i));
        }
    }

    MapperGuard alloc(VirtualAddress addr, bool writable);
};

}  // namespace nyan::paging
