INT_VECTOR_SYSCALL  equ 0x90
IDX_GET_TICKS       equ 0

syscall: 
    call save
    sti
    call [syscall_table + eax*4]
    mov  [esi + EAXREG - P_STACKBASE], eax ; save return value to position of eax in proc->regs

    cli
    ret

get_ticks: 
    ;push ebp
    ;mov ebp, esp

    ;push eax
    mov eax, IDX_GET_TICKS
    int INT_VECTOR_SYSCALL
    ;pop eax

    ;pop ebp
    ;ret
