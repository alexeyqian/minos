[org 0x7c00]
[bits 16] 

BOOT_STACK_BASE equ 0x7c00

jmp short boot_start ; convention, first line of code is a jump
nop                  ; used as a place holder here
%include "fat12_header.inc"   ; fixed position, standard bios parameters block

boot_start:   
    mov ax, cs
    mov ds, ax    
    mov ss, ax   
    mov es, ax 
    mov bp, BOOT_STACK_BASE
    mov sp, bp

    jmp boot_load_loader_bin

%include "boot_load_loader_bin.inc"

times 510-($-$$) db 0
dw 0xaa55