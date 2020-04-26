[org 0x7c00]

KERNEL_OFFSET equ 0x1000
mov [BOOT_DRIVE], dl      ; BIOS stores our boot drive in DL, so it'si
                          ; best to remember this for later.

mov bp, 0x9000            ; Here we set out stack safely away from occupied memory
mov sp, bp

call print_new_line
mov bx, MSG_REAL_MODE
call print_string
call print_new_line

call load_kernel
call switch_to_pm        ; Note that we never return from here

jmp $

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print_string
    call print_new_line

    ; Set-up parameters for our disk_load routine , so 
    ; that we load the first 15 sectors (excluding 
    ; the boot sector) from the boot disk (i.e. our 
    ; kernel code) to address KERNEL_OFFSET

    mov bx, KERNEL_OFFSET
    mov dh, 15                ; Load 15 sectors to ES(0x0000):BX
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

%include "print_string.asm"
%include "disk_load.asm"
%include "gdt.asm"
%include "switch_to_pm.asm"
%include "print_string_pm.asm"

[bits 32]
BEGIN_PM:
    mov ebx, MSG_PROT_MODE
    call print_string_pm
    call KERNEL_OFFSET
    jmp $

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