#include "utils.hpp"

#include "../paging/entry.hpp"
#include "../paging/kernel.hpp"
#include "../task/guard.hpp"
#include "frame.hpp"
#include "load.hpp"
#include "pool.hpp"
#include "slab.hpp"

namespace nyan::allocator {

paging::PhysicalAddress physicalFrameAlloc() {
    task::InterruptGuard guard;

    auto physicalOffset = poolManager->alloc();
    return {PoolManager::pageAt(physicalOffset)};
}

void physicalFrameFree(paging::PhysicalAddress addr) {
    task::InterruptGuard guard;

    auto physicalOffset = PoolManager::pageFor(addr.addr);
    poolManager->free(physicalOffset);
}

paging::VirtualAddress virtualFrameAlloc() {
    task::InterruptGuard guard;

    auto virtualOffset = frameManager->alloc();
    return {FrameManager::frameAt(virtualOffset)};
}

void virtualFrameFree(paging::VirtualAddress addr) {
    task::InterruptGuard guard;

    auto virtualOffset = FrameManager::frameFor(addr.addr);
    frameManager->free(virtualOffset);
}

void* frameAlloc() {
    task::InterruptGuard guard;

    auto physicalAddr = physicalFrameAlloc();
    auto virtualAddr = virtualFrameAlloc();
    paging::kernelPageDirectory.map(
        {
            .pAddr = physicalAddr,
            .vAddr = virtualAddr,
        },
        paging::PTE_Present | paging::PTE_ReadWrite | paging::PTE_User);
    virtualAddr.invlpg();
    return reinterpret_cast<void*>(virtualAddr.addr);
}

void frameFree(void* frame) {
    task::InterruptGuard guard;

    paging::VirtualAddress virtualAddr = {reinterpret_cast<uint32_t>(frame)};
    paging::PhysicalAddress physicalAddr;
    if (!paging::kernelPageDirectory.unmap(virtualAddr, physicalAddr)) {
        arch::kfatal("frameFree unmap failed");
    }
    virtualAddr.invlpg();
    virtualFrameFree(virtualAddr);
    physicalFrameFree(physicalAddr);
}

void* alloc(size_t size, size_t align) {
    task::InterruptGuard guard;
    return slabManager->alloc(std::max(size, align));
}

void free(void* addr) {
    if (!addr) {
        return;
    }
    task::InterruptGuard guard;
    slabManager->free(addr);
}

}  // namespace nyan::allocator
