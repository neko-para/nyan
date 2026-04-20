.section .trampoline, "ax", @progbits

.global __sigreturn_trampoline
__sigreturn_trampoline:
    movl $119, %eax       # SYS_SIGRETURN (你自己定义的号)
    int $0x80
