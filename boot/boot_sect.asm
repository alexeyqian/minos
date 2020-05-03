[org 0x7c00]

RM_STACK_OFFSET equ 0x9000
KERNEL_OFFSET equ 0x1000

mov bp, RM_STACK_OFFSET            ; Here we set out stack safely away from occupied memory
mov sp, bp

mov [BOOT_DRIVE], dl      ; BIOS stores our boot drive in DL, 
                          ; so it's best to remember this for later.

call rmprint_nl
mov bx, MSG_REAL_MODE
call rmprint
call rmprint_nl

call load_kernel_from_disk
call switch_to_pm        ; Note that we never return from here

jmp $

%include "rmprint.inc"
%include "load_kernel_from_disk.inc"
%include "gdt.inc"
%include "switch_to_pm.inc"
%include "pmprint.inc"

; Global variables
BOOT_DRIVE:
    db 0
MSG_REAL_MODE:
    db 'Started in 16 bit Real Mode', 0
MSG_PROT_MODE:
    db 'Successfully loanded in 32 bit Protected Mode', 0
MSG_LOAD_KERNEL:
    db 'Loading kernel into memory', 0

times 510-($-$$) db 0
dw 0xaa55