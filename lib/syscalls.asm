INT_VECTOR_SYSCALL  equ 0x90
IDX_KCALL           equ 0
IDX_SENDREC         equ 1

[bits 32]
[section .text]

; ====================================
; kcall(int function, void* pmsg)
global kcall
kcall:
    
    push ecx
    push edx

    mov eax, IDX_KCALL       
    mov ecx, [esp + 8 + 4] ; function
    mov edx, [esp + 8 + 8] ; pmsg
    int INT_VECTOR_SYSCALL ; this soft interrupt will call syscall

    pop edx
    pop ecx
    ret

; ====================================
; sendrec(int function, int src_dest, KMESSAGE* pmsg)
global sendrec
sendrec:
    push ebx
    push ecx
    push edx

    mov eax, IDX_SENDREC
    mov ebx, [esp + 12 + 4]  ; function
    mov ecx, [esp + 12 + 8]  ; src_dest
    mov edx, [esp + 12 + 12] ; pmsg
    int INT_VECTOR_SYSCALL
    
    pop edx
    pop ecx
    pop ebx
    ret