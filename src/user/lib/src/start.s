.global _start
_start:
    call __init_libc
    call main
    movl %eax, %ebx
    movl $1, %eax
    int $0x80
