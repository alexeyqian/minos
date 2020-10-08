[bits 32]
[section .text]
global syscall 
syscall:  
    call save
    sti                       ; allow maskable interrupts
    push esi
    push dword [p_proc_ready] ; push 4th param  
    push edx                  ; push 3rd param
    push ecx                  ; push 2nd param
    push ebx                  ; push 1rd param
    call [syscall_table + eax*4]
    add esp, 4 * 4  ; clean call stack
    
    pop esi
    mov  [esi + EAXREG - P_STACKBASE], eax ; save return value to position of eax in proc->regs

    cli
    ret