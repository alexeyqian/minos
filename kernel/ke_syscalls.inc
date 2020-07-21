INT_VECTOR_SYSCALL  equ 0x90
IDX_GET_TICKS       equ 0
IDX_WRITE           equ 1
IDX_SENDREC         equ 2
IDX_PRINTX          equ 3

[bits 32]
[section .text]
global syscall
syscall: 
    call save
    sti             ; allow maskable interrupts
    push esi
    push dword [p_proc_ready] ; push proc addr as 1st parameter
    
    push edx
    push ecx        ; push 2nd param
    push ebx        ; push 3rd param
    call [syscall_table + eax*4]
    add esp, 4 * 4  ; clean call stack
    
    pop esi
    mov  [esi + EAXREG - P_STACKBASE], eax ; save return value to position of eax in proc->regs

    cli
    ret

global get_ticks
get_ticks: 
    mov eax, IDX_GET_TICKS
    int INT_VECTOR_SYSCALL   
    ret

global write
write:
    mov eax, IDX_WRITE
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    int INT_VECTOR_SYSCALL
    ret

global sendrec
sendrec:
    mov eax, IDX_SENDREC
    mov ebx, [esp + 4]  ; function
    mov ecx, [esp + 8]  ; src_dest
    mov edx, [esp + 12] ; p_msg
    int INT_VECTOR_SYSCALL
    ret

global printx
printx:
    mov eax, IDX_PRINTX
    mov edx, [esp + 4]
    int INT_VECTOR_SYSCALL
    ret
