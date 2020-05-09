; boot loader stage 1
[org 0x7c00]
[bits 16]

RM_STACK_OFFSET equ 0x9000

; setup segement registers
;mov ax, cs ; not neccessary, since all default to 0
;mov ds, ax
;mov es, ax
;mov ss, ax
mov bp, RM_STACK_OFFSET ; set stack safely away from occupied memory    
mov sp, bp

; BIOS stores our boot drive in dl, 
; so it's best to remember this for later.
mov [boot_drive], dl      

mov bx, msg_start_real_mode
call rm_print_str

jmp $

;call rm_check_mem_map

;call load_kernel_from_disk

;call switch_to_pm

%include "rm_lib.inc"
;%include "rm_check_mem_map.inc"

;%include "rm_load_kernel_from_disk.inc"
;%include "rm_gdt.inc"

;%include "pm_start"

; global variables
boot_drive:
    db 0
msg_start_real_mode:
    db 'start in real mode', 0
times 510-($-$$) db 0
dw 0xaa55