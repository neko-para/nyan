.global switchToTask
.type switchToTask, @function
switchToTask:
    pushfl
    push %ebx
    push %esi
    push %edi
    push %ebp

    # edi = currentTask->userEsp
    movl (currentTask), %edi
    # *edi = esp
    movl %esp, (%edi)

    # esi = nextTask
    movl 24(%esp), %esi
    # currentTask = esi
    movl %esi, (currentTask)

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
