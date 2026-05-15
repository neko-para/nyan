.macro EXCEPTION_NOERRCODE num
.global exception_stub_\num
.type exception_stub_\num, @function
exception_stub_\num:
    .cfi_startproc simple
    .cfi_signal_frame

    .cfi_def_cfa esp, 0
    .cfi_offset eip, 0
    .cfi_offset esp, 12

    pushl $0
    .cfi_adjust_cfa_offset 4
    pushl $\num
    .cfi_adjust_cfa_offset 4
    pushal
    .cfi_adjust_cfa_offset 32

    .cfi_offset edi, -40
    .cfi_offset esi, -36
    .cfi_offset ebp, -32
    .cfi_offset ebx, -24
    .cfi_offset edx, -20
    .cfi_offset ecx, -16
    .cfi_offset eax, -12

    pushw %ds
    .cfi_adjust_cfa_offset 2
    pushw %es
    .cfi_adjust_cfa_offset 2
    pushw %fs
    .cfi_adjust_cfa_offset 2
    pushw %gs
    .cfi_adjust_cfa_offset 2

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl %esp
    .cfi_adjust_cfa_offset 4

    call exceptionHandlerImpl
    .cfi_adjust_cfa_offset 0

    addl $4, %esp
    .cfi_adjust_cfa_offset -4

    popw %gs
    .cfi_adjust_cfa_offset -2
    popw %fs
    .cfi_adjust_cfa_offset -2
    popw %es
    .cfi_adjust_cfa_offset -2
    popw %ds
    .cfi_adjust_cfa_offset -2
    popal
    .cfi_adjust_cfa_offset -32
    addl $8, %esp
    .cfi_adjust_cfa_offset -8
    iret
    .cfi_endproc
.endm

.macro EXCEPTION_ERRCODE num
.global exception_stub_\num
.type exception_stub_\num, @function
exception_stub_\num:
    pushl $\num

    pushal
    pushw %ds
    pushw %es
    pushw %fs
    pushw %gs

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl %esp
    call exceptionHandlerImpl
    addl $4, %esp

    popw %gs
    popw %fs
    popw %es
    popw %ds
    popal
    addl $8, %esp
    iret
.endm

EXCEPTION_NOERRCODE 0      # Division Error
EXCEPTION_NOERRCODE 1      # Debug
EXCEPTION_NOERRCODE 2      # NMI
EXCEPTION_NOERRCODE 3      # Breakpoint
EXCEPTION_NOERRCODE 4      # Overflow
EXCEPTION_NOERRCODE 5      # Bound Range Exceeded
EXCEPTION_NOERRCODE 6      # Invalid Opcode
EXCEPTION_NOERRCODE 7      # Device Not Available
EXCEPTION_ERRCODE   8      # Double Fault
EXCEPTION_NOERRCODE 9      # Coprocessor Segment Overrun
EXCEPTION_ERRCODE   10     # Invalid TSS
EXCEPTION_ERRCODE   11     # Segment Not Present
EXCEPTION_ERRCODE   12     # Stack-Segment Fault
EXCEPTION_ERRCODE   13     # General Protection Fault
EXCEPTION_ERRCODE   14     # Page Fault
EXCEPTION_NOERRCODE 15     # Reserved
EXCEPTION_NOERRCODE 16     # x87 FP Exception
EXCEPTION_ERRCODE   17     # Alignment Check
EXCEPTION_NOERRCODE 18     # Machine Check
EXCEPTION_NOERRCODE 19     # SIMD FP Exception
EXCEPTION_NOERRCODE 20     # Virtualization Exception
EXCEPTION_ERRCODE   21     # Control Protection
EXCEPTION_NOERRCODE 22     # Reserved
EXCEPTION_NOERRCODE 23     # Reserved
EXCEPTION_NOERRCODE 24     # Reserved
EXCEPTION_NOERRCODE 25     # Reserved
EXCEPTION_NOERRCODE 26     # Reserved
EXCEPTION_NOERRCODE 27     # Reserved
EXCEPTION_NOERRCODE 28     # Hypervisor Injection
EXCEPTION_ERRCODE   29     # VMM Communication
EXCEPTION_ERRCODE   30     # Security Exception
EXCEPTION_NOERRCODE 31     # Reserved

EXCEPTION_NOERRCODE 32     # Timer
EXCEPTION_NOERRCODE 33     # Keyboard

EXCEPTION_NOERRCODE 128    # Syscall

.section .rodata
.global exceptionStubs
exceptionStubs:
    .long exception_stub_0
    .long exception_stub_1
    .long exception_stub_2
    .long exception_stub_3
    .long exception_stub_4
    .long exception_stub_5
    .long exception_stub_6
    .long exception_stub_7
    .long exception_stub_8
    .long exception_stub_9
    .long exception_stub_10
    .long exception_stub_11
    .long exception_stub_12
    .long exception_stub_13
    .long exception_stub_14
    .long exception_stub_15
    .long exception_stub_16
    .long exception_stub_17
    .long exception_stub_18
    .long exception_stub_19
    .long exception_stub_20
    .long exception_stub_21
    .long exception_stub_22
    .long exception_stub_23
    .long exception_stub_24
    .long exception_stub_25
    .long exception_stub_26
    .long exception_stub_27
    .long exception_stub_28
    .long exception_stub_29
    .long exception_stub_30
    .long exception_stub_31
