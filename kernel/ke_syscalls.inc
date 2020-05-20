INT_VECTOR_SYS_CALL equ 0x90
IDX_GET_TICKS       equ 0

syscall_handler:
    call save
    sti

    call [syscall_table + eax*4]
    mov [esi + EAXREG - P_STACKBASE], eax ; save return value to position of eax in proc->regs

    cli
    ret

get_ticks:
    mov eax, IDX_GET_TICKS
    int INT_VECTOR_SYS_CALL
    ret
