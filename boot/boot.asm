[org 0x7c00]
[bits 16] 
; since it will compile to 'bin' format, so bits 16 is not supported/not used.
; here just use it as a good practise.

%include "constants.inc"
BOOT_STACK_BASE equ 0x7c00

; ============== begin of fix order ===============
jmp short boot_start                        ; convention, first line of code is a jump
nop                                         ; used as a place holder here
%include "fixed_bios_parameter_block.inc"   ; standard bios parameters block (BPB)
; ============== end of fix order =================

boot_start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov bp, BOOT_STACK_BASE
    mov sp, bp

    jmp rm_load_loader

%include "rm_lib.inc"
%include "rm_read_sectors.inc"
%include "rm_get_fat_entry.inc"
%include "rm_load_loader.inc"

times 510-($-$$) db 0
dw 0xaa55