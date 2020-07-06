[org 0x7c00]
[bits 16] 

BOOT_STACK_BASE equ 0x7c00

; ============== begin of fix order ===============
jmp short boot_start                        ; convention, first line of code is a jump
nop                                         ; used as a place holder here
%include "fixed_bios_parameter_block.inc"   ; standard bios parameters block (BPB), FAT12HEADER, for reading FAT12 floppy
; ============== end of fix order =================

boot_start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov bp, BOOT_STACK_BASE
    mov sp, bp

    jmp rm_load_loader

%include "rm_load_loader.inc"     ; fixed location

times 510-($-$$) db 0
dw 0xaa55