#pragma once

#include <stdint.h>

namespace nyan::interrupt {

struct Entry;

struct Frame {
    uint32_t eip;
    uint32_t cs;
    uint32_t flags;
};

struct SyscallFrame {
    uint16_t user_gs;
    uint16_t user_fs;
    uint16_t user_es;
    uint16_t user_ds;

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t _esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t eip;
    uint32_t cs;
    uint32_t flags;
    uint32_t user_esp;
    uint32_t user_ss;
};

enum Exception {
    E_DivisionError,
    E_Debug,
    E_NonMaskableInterrupt,
    E_Breakpoint,
    E_Overflow,
    E_BoundRangeExceeded,
    E_InvalidOpcode,
    E_DeviceNotAvailable,
    E_DoubleFault,
    E_CoprocessorSegmentOverrun,
    E_InvalidTSS,
    E_SegmentNotPresent,
    E_StackSegmentFault,
    E_GeneralProtectionFault,
    E_PageFault,

    E_X87FloatingPointException = 16,
    E_AligmentCheck,
    E_MachineCheck,
    E_SIMDFloatingPointException,
    E_VirtualizationException,
    E_ControlProtectionException,

    E_HypervisorInjectionException = 28,
    E_VMMCommunicationException,
    E_SecurityException,
};

enum PageFault {
    PF_Present = 1 << 0,
    PF_Write = 1 << 1,
    PF_User = 1 << 2,
    PF_ReservedWrite = 1 << 3,
    PF_InstructionFetch = 1 << 4,
    PF_ProtectionKey = 1 << 5,
    PF_ShadowStack = 1 << 6,
    PF_SoftwareGuardExtensions = 1 << 15,
};

template <uint32_t Id>
__attribute__((interrupt)) void defaultHandler(Frame*, uint32_t error);

template <uint32_t Id>
__attribute__((interrupt)) void defaultHandlerNe(Frame*);

template <uint32_t Id>
void defaultHandlerImpl(Frame*, uint32_t error);

template <uint32_t Id>
void defaultHandlerImplNe(Frame*);

extern "C" void syscallHandler();
extern "C" void syscallHandlerImpl(SyscallFrame* frame);

void fillEntries(Entry* entry);

}  // namespace nyan::interrupt
