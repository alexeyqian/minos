INT_VECTOR_SYSCALL  equ 0x90
IDX_PRINTX          equ 0
IDX_SENDREC         equ 1

[bits 32]
[section .text]

; void printx(char* s)
global printx
printx:
    push edx

    mov eax, IDX_PRINTX
    mov edx, [esp + 4 + 4] ; param: s
    int INT_VECTOR_SYSCALL

    pop edx

    ret

; sendrec(int function, int src_dest, MESSAGE* pmsg)
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