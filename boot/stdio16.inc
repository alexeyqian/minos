%ifndef _STDIO16_INC_
%define _STDIO16_INC_

;************************************************;
;   16 bits functions
;************************************************;
bits 16

; puts16()
; print a null terminated string
; ds:si: 0 terminated string

puts16:
    pusha
.loop:
    lodsb     ; load next byte from string from ds:si to al
    or al, al ; does al = 0?
    jz .done  ; null terminator found, bail out
    mov ah, 0eh
    int 10h
    jmp .loop
.done:
    popa
    ret
%endif