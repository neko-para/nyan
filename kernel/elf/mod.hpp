#pragma once

#include <span>

#include "../paging/address.hpp"
#include "../paging/directory.hpp"
#include "../paging/vma.hpp"

namespace nyan::elf {

struct LoadResult {
    paging::UserDirectory pageDir;
    paging::VirtualAddress brkAddr;
    paging::VirtualAddress entry;
};

LoadResult loadElf(paging::VMSpace& vmSpace, std::span<uint8_t> file) noexcept;

}  // namespace nyan::elf