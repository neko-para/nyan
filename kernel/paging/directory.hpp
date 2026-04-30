#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../arch/print.hpp"
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
    uint16_t attr(uint16_t location) const noexcept { return data[location] & 0xFFF; }
    bool isPresent(uint16_t location) const noexcept { return data[location] & PDE_Present; }
};

struct KernelDirectory : public DirectoryData {
    void map(VirtualAddress vAddr, PhysicalAddress pAddr, uint16_t attr) const noexcept {
        auto table = at(vAddr.tableLoc());
        if (!table) {
            arch::kfatal("table not exists");
        }
        table.kernelToVirtual().as<Table>()->map(vAddr, pAddr, attr);
    }

    bool unmap(VirtualAddress virtualAddr, PhysicalAddress& physicalAddr) const noexcept {
        auto table = at(virtualAddr.tableLoc());
        if (!table) {
            arch::kfatal("table not exists");
        }
        return table.kernelToVirtual().as<Table>()->unmap(virtualAddr, physicalAddr);
    }

    PhysicalAddress cr3() const noexcept { return paging::VirtualAddress{this}.kernelToPhysical(); }
};

struct MapperGuard {
    MapperGuard(PhysicalAddress addr) noexcept;
    MapperGuard(const MapperGuard&) = delete;
    MapperGuard(MapperGuard&& mapper) noexcept : paddr(mapper.paddr), vaddr(mapper.vaddr.addr) {
        mapper.paddr = 0_pa;
        mapper.vaddr = 0_va;
    }
    ~MapperGuard();
    MapperGuard& operator=(const MapperGuard&) = delete;
    MapperGuard& operator=(MapperGuard&& mapper) noexcept {
        if (this == &mapper) {
            return *this;
        }
        this->~MapperGuard();
        paddr = mapper.paddr;
        vaddr = mapper.vaddr;
        mapper.paddr = 0_pa;
        mapper.vaddr = 0_va;
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
    MapperGuard __mapper;

    DirectoryData* data() const noexcept { return __mapper.as<DirectoryData>(); }
    DirectoryData* operator->() const noexcept { return data(); }

    static UserDirectory from(PhysicalAddress addr) noexcept { return {{addr}}; }
    static UserDirectory fork(const KernelDirectory& directory) noexcept;
    static UserDirectory forkCOW(const UserDirectory& directory) noexcept;

    void ensure(uint16_t location, uint16_t attr) noexcept;

    template <typename Func>
    auto with(uint16_t location, Func func) const noexcept {
        MapperGuard mapper(data()->at(location));
        return func(mapper.as<Table>());
    }

    void freePage(VirtualAddress addr) noexcept;
    void freePageTables() noexcept;
    MapperGuard alloc(VirtualAddress addr, bool writable) noexcept;
    bool handleCOW(VirtualAddress addr) noexcept;
};

}  // namespace nyan::paging
