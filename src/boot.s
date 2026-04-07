.set MAGIC, 0x1BADB002
.set FLAGS, 3

.section .multiboot.data, "a"
.align 4
.long MAGIC
.long FLAGS
.long -(MAGIC + FLAGS)

.section .bss
.global stack_top
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .multiboot.text, "a"
.global _start, enableSse
.type _start, @function
_start:
    movl $(stack_top - 0xC0000000), %esp
    pushl %ebx

    movl $(preparePaging - 0xC0000000), %ecx
    call *%ecx

    addl $0xC0000000, %esp
    addl $0xC0000000, %ebp

    lea 1f, %ecx
    jmp *%ecx

.section .text
1:
    call kmain

    cli
2:
    hlt
    jmp 2b
