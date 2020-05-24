INT_VECTOR_SYSCALL  equ 0x90
IDX_GET_TICKS       equ 0
IDX_WRITE           equ 1

[bits 32]
[section .text]
syscall: 
    call save
    push dword [p_proc_ready] ; bus proc addr as 1st parameter
    sti             ; allow maskable interrupts
    push ecx        ; push 2nd param
    push ebx        ; push 3rd param
    call [syscall_table + eax*4]
    add esp, 4 * 3  ; clean call stack
    
    mov  [esi + EAXREG - P_STACKBASE], eax ; save return value to position of eax in proc->regs

    cli
    ret

get_ticks: 
    mov eax, IDX_GET_TICKS
    int INT_VECTOR_SYSCALL   
    ret

write:
    mov eax, IDX_WRITE
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    int INT_VECTOR_SYSCALL
    ret
