.global syscallHandler
.type syscallHandler, @function
syscallHandler:
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
    call syscallHandlerImpl 
    addl $4, %esp

    popw %gs
    popw %fs
    popw %es
    popw %ds
    popal
    iret

.global timerHandler
.type timerHandler, @function
timerHandler:
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
    call timerHandlerImpl 
    addl $4, %esp

    popw %gs
    popw %fs
    popw %es
    popw %ds
    popal
    iret
