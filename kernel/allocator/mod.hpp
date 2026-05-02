#pragma once

#include "../paging/address.hpp"

namespace nyan::allocator {

paging::PhysicalAddress physicalFrameAlloc() noexcept;
void physicalFrameRetain(paging::PhysicalAddress addr) noexcept;
void physicalFrameRelease(paging::PhysicalAddress addr) noexcept;
uint16_t physicalFrameGetRef(paging::PhysicalAddress addr) noexcept;

paging::VirtualAddress virtualFrameAlloc() noexcept;
void virtualFrameFree(paging::VirtualAddress addr) noexcept;

void* frameAlloc() noexcept;
void frameFree(void* frame) noexcept;
void* largeFrameAlloc(size_t page) noexcept;
void largeFrameFree(void* frame) noexcept;

void* autoAlloc(size_t size) noexcept;
void autoFree(void* frame) noexcept;

void load(uint32_t upper) noexcept;

}  // namespace nyan::allocator