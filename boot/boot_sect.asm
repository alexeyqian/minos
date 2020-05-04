[org 0x7c00]
[bits 16]

RM_STACK_OFFSET equ 0x9000
KERNEL_OFFSET equ 0x1000

; Here we set out stack safely away from occupied memory
mov bp, RM_STACK_OFFSET            
mov sp, bp

; BIOS stores our boot drive in DL, 
; so it's best to remember this for later.
mov [BOOT_DRIVE], dl      

; Print real mode message
;call rmprint_nl
;mov bx, MSG_REAL_MODE
;call rmprint
;call rmprint_nl

call load_kernel_from_disk

; Switch to protected mode
cli
lgdt [gdt_ptr]

mov eax, cr0
or eax, 0x1
mov cr0, eax

jmp CODE_SEG:init_pm ; Make a far jump.
                     ; This also forces the CPU to flush its cache
                     ; of pre-fetched and real mode decoded instructions, 
                     ; which can cause problem.

%include "rmprint.inc"
%include "load_kernel_from_disk.inc"
%include "gdt.inc"

; ===================== enter 32 bits mode =================                         
[bits 32]

PM_STACK_OFFSET equ 0xa000

; Initialise registers and the stack once in init_pm
init_pm:
    mov ax, DATA_SEG  ; Now in PM, out old segments are meaningless
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, PM_STACK_OFFSET
    mov esp, ebp
    
    call begin_pm

begin_pm:
    mov ebx, MSG_PROT_MODE
    call print_string_pm
    
    call KERNEL_OFFSET
    ; should never return to here
    jmp $  

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