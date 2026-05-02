#pragma once

#include <stdint.h>
#include <sys/types.h>

namespace nyan::interrupt {

struct Entry;

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

    uint32_t isr_num;
    uint32_t error_code;

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

    E_LastException = E_SecurityException,

    I_Timer = 32,
    I_Keyboard = 33,
    I_Syscall = 128,
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

extern uint32_t exceptionStubs[32] asm("exceptionStubs");

extern "C" void exception_stub_32();
extern "C" void exception_stub_33();
extern "C" void exception_stub_128();

extern "C" void exceptionHandlerImpl(SyscallFrame* frame);
extern "C" void syscallHandlerImpl(SyscallFrame* frame);

void fillEntries(Entry* entry);

}  // namespace nyan::interrupt
