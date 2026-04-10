#include "utils.hpp"

#include "../paging/entry.hpp"
#include "../paging/kernel.hpp"
#include "../task/guard.hpp"
#include "frame.hpp"
#include "load.hpp"
#include "pool.hpp"
#include "slab.hpp"

namespace nyan::allocator {

void* frameAlloc() {
    task::InterruptGuard guard;

    auto physicalOffset = poolManager->alloc();
    auto physicalAddr = PoolManager::pageAt(physicalOffset);
    auto virtualOffset = frameManager->alloc();
    auto virtualAddr = FrameManager::frameAt(virtualOffset);
    paging::kernelPageDirectory.map(physicalAddr, virtualAddr, paging::PDE_Present | paging::PDE_ReadWrite);
    return reinterpret_cast<void*>(virtualAddr);
}

void frameFree(void* frame) {
    task::InterruptGuard guard;

    auto virtualAddr = reinterpret_cast<uint32_t>(frame);
    uint32_t physicalAddr;
    if (!paging::kernelPageDirectory.unmap(virtualAddr, physicalAddr)) {
        arch::kfatal("frameFree unmap failed");
    }
    auto virtualOffset = FrameManager::frameFor(virtualAddr);
    auto physicalOffset = PoolManager::pageFor(physicalAddr);
    frameManager->free(virtualOffset);
    poolManager->free(physicalOffset);
}

void* alloc(size_t size) {
    task::InterruptGuard guard;
    return slabManager->alloc(size);
}

void free(void* addr) {
    task::InterruptGuard guard;
    slabManager->free(addr);
}

}  // namespace nyan::allocator
