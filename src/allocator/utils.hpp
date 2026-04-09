#pragma once

namespace nyan::allocator {

void* frameAlloc();
void frameFree(void* frame);

}  // namespace nyan::allocator
