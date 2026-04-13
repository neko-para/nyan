#pragma once

#include <stdint.h>

namespace nyan::elf {

enum Bit : uint8_t {
    B_32 = 1,
    B_64 = 2,
};

enum Endian : uint8_t {
    E_Little = 1,
    E_Big = 2,
};

enum OSABI : uint8_t {
    OA_SystemV = 0,
};

enum Type : uint16_t {
    T_Relocatable = 1,
    T_Executable = 2,
    T_Shared = 3,
    T_Core = 4,
};

enum InstructionSet : uint16_t {
    IS_x86 = 0x03,
    IS_ARM = 0x28,
    IS_x86_64 = 0x3e,
    IS_AArch64 = 0xb7,
};

struct Header {
    char magic[4];
    Bit bit;
    Endian endian;
    uint8_t header_version;
    OSABI abi;
    uint8_t reserved[8];
    Type type;
    InstructionSet instruction_set;
    uint32_t elf_version;  // 1
    uint32_t entry_offset;
    uint32_t program_header_table_offset;
    uint32_t section_header_table_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_entry_count;
    uint16_t section_header_entry_size;
    uint16_t section_header_entry_count;
    uint16_t string_table_section_index;
};

enum PHType : uint32_t {
    PHT_Null = 0,
    PHT_Load = 1,
    PHT_Dynamic = 2,
    PHT_Interp = 3,
    PHT_Note = 4,
};

enum PHFlag {
    PHF_Executable = 1,
    PHF_Writable = 2,
    PHF_Readable = 4,
};

struct ProgramHeader {
    PHType type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
};

}  // namespace nyan::elf
