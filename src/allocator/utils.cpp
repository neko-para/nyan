#include "utils.hpp"

#include "../paging/entry.hpp"
#include "../paging/kernel.hpp"
#include "../task/guard.hpp"
#include "frame.hpp"
#include "load.hpp"
#include "pool.hpp"
#include "slab.hpp"

namespace nyan::allocator {

uint32_t physicalFrameAlloc() {
    task::InterruptGuard guard;

    auto physicalOffset = poolManager->alloc();
    return PoolManager::pageAt(physicalOffset);
}

void physicalFrameFree(uint32_t addr) {
    task::InterruptGuard guard;

    auto physicalOffset = PoolManager::pageFor(addr);
    poolManager->free(physicalOffset);
}

uint32_t virtualFrameAlloc() {
    task::InterruptGuard guard;

    auto virtualOffset = frameManager->alloc();
    return FrameManager::frameAt(virtualOffset);
}

void virtualFrameFree(uint32_t addr) {
    task::InterruptGuard guard;

    auto virtualOffset = FrameManager::frameFor(addr);
    frameManager->free(virtualOffset);
}

void* frameAlloc() {
    task::InterruptGuard guard;

    auto physicalAddr = physicalFrameAlloc();
    auto virtualAddr = virtualFrameAlloc();
    paging::kernelPageDirectory.map(physicalAddr, virtualAddr,
                                    paging::PTE_Present | paging::PTE_ReadWrite | paging::PTE_User);
    return reinterpret_cast<void*>(virtualAddr);
}

void frameFree(void* frame) {
    task::InterruptGuard guard;

    auto virtualAddr = reinterpret_cast<uint32_t>(frame);
    uint32_t physicalAddr;
    if (!paging::kernelPageDirectory.unmap(virtualAddr, physicalAddr)) {
        arch::kfatal("frameFree unmap failed");
    }
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
