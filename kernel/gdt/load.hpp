#pragma once

#include <sys/types.h>

namespace nyan::gdt {

struct Segment;

constexpr size_t kernelCs = 0x08;
constexpr size_t kernelDs = 0x10;
constexpr size_t userCs = 0x18 | 0x3;
constexpr size_t userDs = 0x20 | 0x3;

inline bool isRing0(size_t seg) noexcept {
    return (seg & 0x3) == 0;
}
inline bool isRing3(size_t seg) noexcept {
    return (seg & 0x3) == 3;
}

void load();
void setTls(const Segment& seg);

}  // namespace nyan::gdt
