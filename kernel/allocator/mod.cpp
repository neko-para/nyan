#include "mod.hpp"

#include "../arch/guard.hpp"
#include "../paging/directory.hpp"
#include "forward.hpp"
#include "frame.hpp"
#include "large_frame.hpp"
#include "physical_frame.hpp"
#include "pool.hpp"
#include "slab.hpp"

namespace nyan::allocator {

paging::PhysicalAddress physicalFrameAlloc() noexcept {
    arch::InterruptGuard guard;

    auto offset = __pool_manager->alloc();
    __physical_frame_manager->__info[offset] = {.__ref = 1};
    return paging::PhysicalAddress{PoolManager::pageAt(offset)};
}

void physicalFrameRetain(paging::PhysicalAddress addr) noexcept {
    arch::InterruptGuard guard;

    auto offset = PoolManager::pageFor(addr.addr);
    __physical_frame_manager->__info[offset].__ref += 1;
}

void physicalFrameRelease(paging::PhysicalAddress addr) noexcept {
    arch::InterruptGuard guard;

    auto offset = PoolManager::pageFor(addr.addr);
    __physical_frame_manager->__info[offset].__ref -= 1;
    if (!__physical_frame_manager->__info[offset].__ref) {
        __pool_manager->free(offset);
    }
}

uint16_t physicalFrameGetRef(paging::PhysicalAddress addr) noexcept {
    arch::InterruptGuard guard;

    auto offset = PoolManager::pageFor(addr.addr);
    return __physical_frame_manager->__info[offset].__ref;
}

paging::VirtualAddress virtualFrameAlloc() noexcept {
    arch::InterruptGuard guard;

    auto virtualOffset = __frame_manager->alloc();
    return paging::VirtualAddress{FrameManager::frameAt(virtualOffset)};
}

void virtualFrameFree(paging::VirtualAddress addr) noexcept {
    arch::InterruptGuard guard;

    auto virtualOffset = FrameManager::frameFor(addr.addr);
    __frame_manager->free(virtualOffset);
}

void* frameAlloc() noexcept {
    arch::InterruptGuard guard;

    auto physicalAddr = physicalFrameAlloc();
    auto virtualAddr = virtualFrameAlloc();
    paging::kernelPageDirectory.map(virtualAddr, physicalAddr, paging::PTE_Present | paging::PTE_ReadWrite);
    virtualAddr.invlpg();
    memset(virtualAddr.as<void>(), 0, 4096);
    return virtualAddr.as<void>();
}

void frameFree(void* frame) noexcept {
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

void* largeFrameAlloc(size_t page) noexcept {
    arch::InterruptGuard guard;

    auto addr = __large_frame_manager->alloc(page);
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

void largeFrameFree(void* frame) noexcept {
    arch::InterruptGuard guard;

    paging::VirtualAddress addr{frame};
    auto page = __large_frame_manager->free(addr);
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

void* autoAlloc(size_t size) noexcept {
    if (size <= 512) {
        arch::InterruptGuard guard;
        return __slab_manager->alloc(size);
    } else if (size <= 4096) {
        return frameAlloc();
    } else {
        return largeFrameAlloc((size + 0xFFF) >> 12);
    }
}

void autoFree(void* frame) noexcept {
    if (!frame) {
        return;
    }
    auto addr = reinterpret_cast<uint32_t>(frame);

    if (addr >= __large_frame_base) {
        largeFrameFree(frame);
    } else if (addr & 0xFFF) {
        arch::InterruptGuard guard;
        __slab_manager->free(frame);
    } else {
        frameFree(frame);
    }
}

}  // namespace nyan::allocator