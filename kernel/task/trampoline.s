.section .trampoline, "ax", @progbits

.global __sigreturn_trampoline
__sigreturn_trampoline:
    popl %eax
    movl $119, %eax
    int $0x80
