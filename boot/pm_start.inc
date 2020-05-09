; enter 32 bits mode -------                
[bits 32]
KERNEL_OFFSET equ 0x1000
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

;%include "pmprint.inc"

; global variable
MSG_PROT_MODE:
    db 'Successfully loanded in 32 bit Protected Mode', 0
MSG_LOAD_KERNEL:
    db 'Loading kernel into memory', 0