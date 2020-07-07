%ifndef _LOADER_LIB_
%define _LOADER_LIB_

[bits 16]
;========== function loader_putax: print char at top left =======
; ax: char to print
; same as boot_putax, but duplidate here by design to avoid sharing files between boot and loader
; since they will be compiled into 2 seperate binary files.
loader_putax:    
    push bx ; save bx
    push es

    mov ah, 0x0f ; white on black
    mov bx, 0xB800
    mov es, bx
    mov [es:0x0], ax

    pop es
    pop bx ; restore bx
    ret
    
%endif