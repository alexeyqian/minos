; GDT will also be moved from loader to kernel for easy control

[bits 32]
%include "ke_constants.inc"
%include "ke_imports.inc"

[section .text]
global _start
_start:  
	mov ebp, kernel_stack_top ; setup kernel stack
	mov esp, kernel_stack_top 
	call kinit                 ; never return
	
%include "ke_interrupts.inc"
%include "ke_syscalls.inc"
%include "ke_asm_utils.inc"

[section .bss]
stack_space resb 2*1024 ; reserved 2K for kernle stack
kernel_stack_top: 