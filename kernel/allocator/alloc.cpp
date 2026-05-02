#include "alloc.hpp"

#include "../arch/guard.hpp"
#include "../paging/directory.hpp"
#include "../paging/entry.hpp"
#include "frame.hpp"
#include "large_frame.hpp"
#include "load.hpp"
#include "physicalFrame.hpp"
#include "pool.hpp"
#include "slab.hpp"

namespace nyan::allocator {

paging::PhysicalAddress physicalFrameAlloc() {
    arch::InterruptGuard guard;

    auto offset = poolManager->alloc();
    physicalFrameManager->info[offset] = {.ref = 1};
    return paging::PhysicalAddress{PoolManager::pageAt(offset)};
}

void physicalFrameRetain(paging::PhysicalAddress addr) {
    arch::InterruptGuard guard;

    auto offset = PoolManager::pageFor(addr.addr);
    physicalFrameManager->info[offset].ref += 1;
}

void physicalFrameRelease(paging::PhysicalAddress addr) {
    arch::InterruptGuard guard;

    auto offset = PoolManager::pageFor(addr.addr);
    physicalFrameManager->info[offset].ref -= 1;
    if (!physicalFrameManager->info[offset].ref) {
        poolManager->free(offset);
    }
}

uint16_t physicalFrameGetRef(paging::PhysicalAddress addr) {
    arch::InterruptGuard guard;

    auto offset = PoolManager::pageFor(addr.addr);
    return physicalFrameManager->info[offset].ref;
}

paging::VirtualAddress virtualFrameAlloc() {
    arch::InterruptGuard guard;

    auto virtualOffset = frameManager->alloc();
    return paging::VirtualAddress{FrameManager::frameAt(virtualOffset)};
}

void virtualFrameFree(paging::VirtualAddress addr) {
    arch::InterruptGuard guard;

    auto virtualOffset = FrameManager::frameFor(addr.addr);
    frameManager->free(virtualOffset);
}

void* frameAlloc() {
    arch::InterruptGuard guard;

    auto physicalAddr = physicalFrameAlloc();
    auto virtualAddr = virtualFrameAlloc();
    paging::kernelPageDirectory.map(virtualAddr, physicalAddr, paging::PTE_Present | paging::PTE_ReadWrite);
    virtualAddr.invlpg();
    memset(virtualAddr.as<void>(), 0, 4096);
    return virtualAddr.as<void>();
}

void frameFree(void* frame) {
    arch::InterruptGuard guard;

    paging::VirtualAddress virtualAddr = paging::VirtualAddress{frame};
    paging::PhysicalAddress physicalAddr;
    if (!paging::kernelPageDirectory.unmap(virtualAddr, physicalAddr)) {
        arch::kfatal("frameFree unmap failed");
    }
    virtualAddr.invlpg();
    virtualFrameFree(virtualAddr);
    physicalFrameRelease(physicalAddr);
}

void* slabAlloc(size_t size, size_t align) {
    arch::InterruptGuard guard;
    return slabManager->alloc(std::max(size, align));
}

void slabFree(void* addr) {
    if (!addr) {
        return;
    }
    arch::InterruptGuard guard;
    slabManager->free(addr);
}

void* largeFrameAlloc(size_t page) {
    arch::InterruptGuard guard;

    auto addr = largeFrameManager->alloc(page);
    if (!addr) {
        arch::kfatal("large frame alloc failed");
    }
    auto curr = *addr;
    for (size_t i = 0; i < page; i++) {
        auto pAddr = physicalFrameAlloc();
        paging::kernelPageDirectory.map(curr, pAddr, paging::PTE_Present | paging::PTE_ReadWrite);
        curr.invlpg();
        memset(curr.as<void>(), 0, 4096);
        curr = curr.nextPage();
        // arch::kprint("large alloc {#10x}+{} -> {#10x}", addr->addr, i, pAddr.addr);
    }

    return addr->as<void>();
}

void largeFrameFree(void* frame) {
    arch::InterruptGuard guard;

    paging::VirtualAddress addr{frame};
    auto page = largeFrameManager->free(addr);
    arch::kprint("large free {#10x} {}", addr.addr, page);
    for (size_t i = 0; i < page; i++) {
        paging::PhysicalAddress pAddr;
        if (!paging::kernelPageDirectory.unmap(addr, pAddr)) {
            arch::kfatal("largeFrameFree unmap failed");
        }
        addr.invlpg();
        physicalFrameRelease(pAddr);
        addr = addr.nextPage();
    }
}

}  // namespace nyan::allocator
