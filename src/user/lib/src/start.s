.global _start
_start:
    call __init_libc
    call main
    push %eax
    call exit
