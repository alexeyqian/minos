%ifndef _RM_LIB_
%define _RM_LIB_

[bits 16]
; ---------------------------------------------------------
; print string end with '\0'
; string address in bx
; ---------------------------------------------------------

rm_print_str:
    push bp
    mov bp, sp

    push ax 
    push bx

    mov ah, 0x0e

.loop:
    mov al, [bx]
    cmp al, 0
    je .done
    
    int 0x10
    add bx, 1
    jmp .loop

.done:
    pop bx
    pop ax

    mov sp, bp
    pop bp
    ret

; ---------------------------------------------------------

; ---------------------------------------------------------
; print new line
; ---------------------------------------------------------
rm_print_nl:
    push bp
    mov bp, sp

    push ax

    mov ah, 0x0e
    mov al, 0x0d
    int 0x10
    mov al, 0x0a
    int 0x10

    pop ax

    mov sp, bp
    pop bp
    ret
; ---------------------------------------------------------

%endif