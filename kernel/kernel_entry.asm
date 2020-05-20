; kernel is first read to at address:0x8000 from disk by loader in real mode 
; then it's been moved to address:0x30000 from 0x8000 to execute in protection mode.
; the reason for read first then move, is because is easy to read from disk in real mode which has BIOS.
; but the kernel is elf format, and cannot run directly as pure bin, since it has format data.
; so it need to parse the elf file and load/move 'segments' into memory, so it can execute. 
; in protected mode, read from disk is hard, but read from memory is easy.
; so read the file data from disk to memory as buffer for disk data, then parse the data in memory and prepare for execution.

; esp, and GDT will also be moved from loader to kernel for easy control

%include "ke_constants.inc"
%include "ke_imports.inc"

[bits 32]
[section .data]
msg_clock_int db "^", 0

[section .bss]
stack_space resb 2*1024 ; reserved 2K for kernle stack
kernel_stack_top: 

[section .text]
%include "ke_exports.inc"

_start:  
	mov esp, kernel_stack_top ; move esp from loader to kernel

	sgdt [gdt_ptr]            ; for moving gdt
	call kinit                ; move gdt and init idt, tss, proc_table inside
	lgdt [gdt_ptr]            ; reload gdt with at new mem location.
	lidt [idt_ptr]

	jmp KERNEL_SELECTOR:_main

_main:	
	xor eax, eax
	mov ax, SELECTOR_TSS      ; load tss seletor
	ltr ax                    ; which points to a tss descriptor in gdt

	jmp kmain

%include "ke_interrupts.inc"
%include "ke_syscalls.inc"