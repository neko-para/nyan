.global switchToTask
.type switchToTask, @function
switchToTask:
    pushfl
    push %ebx
    push %esi
    push %edi
    push %ebp

    # edi = __scheduler.__current->userEsp
    movl (scheduler), %edi
    # *edi = esp
    movl %esp, (%edi)

    # esi = nextTask
    movl 24(%esp), %esi
    # __scheduler.__current = esi
    movl %esi, (scheduler)

    # esp = esi->userEsp
    movl (%esi), %esp
    # eax = esi->cr3
    movl 4(%esi), %eax
    # ebx = esi->kernelEsp
    movl 8(%esi), %ebx
    # tss->esp0 = ebx
    movl %ebx, (tss + 4)
    # ecx = cr3
    movl %cr3, %ecx

    cmpl %ecx, %eax
    je .done
    mov %eax, %cr3

.done:
    popl %ebp
    popl %edi
    popl %esi
    popl %ebx
    popfl
    ret

.global syscallReturn
.type syscallReturn, @function
syscallReturn:
    movl 4(%esp), %esp

    popw %gs
    popw %fs
    popw %es
    popw %ds
    popal
    addl $8, %esp
    iret


