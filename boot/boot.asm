; boot loader stage 1

;%define _BOOT_DEBUG_

%ifdef _BOOT_DEBUG_
[org 0x100]
%else
[org 0x7c00]
%endif

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

    jmp rm_load_loader

%include "rm_lib.inc"
%include "rm_read_sectors.inc"
%include "rm_get_fat_entry.inc"
%include "rm_load_loader.inc"

times 510-($-$$) db 0
dw 0xaa55