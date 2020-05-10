; boot loader stage 1
[org 0x7c00]
[bits 16]

RM_STACK_BASE equ 0x7c00

jmp short boot_start
nop
%include "fixed_bios_parameter_block.inc" ; has to be included here.

boot_start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov bp, RM_STACK_BASE
    mov sp, bp

    ; BIOS stores our boot drive in dl, 
    ; so it's best to remember this for later.
    mov [boot_drive], dl      

    mov bx, msg_start_real_mode
    call rm_print_str

    call rm_read_loader

    jmp $

%include "rm_lib.inc"
%include "rm_read_loader.inc"

; global variables
boot_drive:
    db 0
msg_start_real_mode:
    db 'start in real mode', 0
times 510-($-$$) db 0
dw 0xaa55