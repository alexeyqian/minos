[bits 16]

; Switch to protected mode
switch_to_pm:
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm ; Make a far jump.
                         ; This also forces the CPU to flush its cache
                         ; of pre-fetched and real mode decoded instructions, 
                         ; which can cause problem.

[bits 32]
; Initialise registers and the stack once in init_pm
init_pm:
    mov ax, DATA_SEG  ; Now in PM, out old segments are meaningless
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, KERNEL_STACK_OFFSET
    mov esp, ebp

    call begin_pm 

begin_pm:
    mov ebx, MSG_PROT_MODE
    call print_string_pm
    call KERNEL_OFFSET
    jmp $