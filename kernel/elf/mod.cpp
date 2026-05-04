#include "mod.hpp"

#include "entry.hpp"

namespace nyan::elf {

LoadResult loadElf(paging::VMSpace& vmSpace, std::span<uint8_t> file) noexcept {
    auto header = std::launder(reinterpret_cast<elf::Header*>(file.data()));
    // TODO: check if support

    auto pageDir = paging::UserDirectory::fork(paging::kernelPageDirectory);

    auto offset = header->program_header_table_offset;
    auto brkAddr = 0x400000_va;
    for (size_t i = 0; i < header->program_header_entry_count; i++) {
        auto program_header = new (file.data() + offset) elf::ProgramHeader;
        offset += header->program_header_entry_size;
        if (program_header->type != elf::PHT_Load) {
            continue;
        }
        if (program_header->align != 0x1000) {
            continue;
        }
        auto lower = paging::VirtualAddress{program_header->vaddr};
        auto upper = paging::VirtualAddress{program_header->vaddr + program_header->memsz};
        auto fileUpper = paging::VirtualAddress{program_header->vaddr + program_header->filesz};
        auto lowerPage = lower.alignDown();
        auto upperPage = upper.alignUp();
        brkAddr = std::max(brkAddr, upperPage);
        for (auto vaddr = lowerPage; vaddr != upperPage; vaddr = vaddr.nextPage()) {
            auto mapper = pageDir.alloc(vaddr, program_header->flags & elf::PHF_Writable);
            auto frame = mapper.as<uint8_t>();

            auto lower_bound = std::max(lower, vaddr);
            auto upper_bound = std::min(fileUpper, vaddr.nextPage());
            if (lower_bound < upper_bound) {
                std::copy_n(&file[program_header->offset + (lower_bound - lower)], upper_bound - lower_bound,
                            &frame[lower_bound - vaddr]);
            }
        }
        paging::VMA vma;
        vma.__begin = lowerPage;
        vma.__end = upperPage;
        vma.__protect = 0;
        if (program_header->flags & elf::PHF_Executable) {
            vma.__protect |= PROT_EXEC;
        }
        if (program_header->flags & elf::PHF_Writable) {
            vma.__protect |= PROT_WRITE;
        }
        if (program_header->flags & elf::PHF_Readable) {
            vma.__protect |= PROT_READ;
        }
        vma.__flags = MAP_PRIVATE | MAP_ANONYMOUS;
        vma.__name = "elf segs";
        vmSpace.insert(vma);
    }

    return {std::move(pageDir), brkAddr, paging::VirtualAddress{header->entry_offset}};
}

}  // namespace nyan::elf