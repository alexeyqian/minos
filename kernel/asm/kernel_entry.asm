; GDT will also be moved from loader to kernel for easy control

[bits 32]
%include "ke_constants.inc"
%include "ke_imports.inc"

[section .text]
global _start
_start:  
	mov ebp, kernel_stack_top ; setup kernel stack
	mov esp, kernel_stack_top 
	call kstart                 ; never return
	
%include "ke_interrupts.inc"
%include "ke_syscalls.inc"
%include "ke_asm_utils.inc"

[section .bss] 
; The stack on x86 must be 16-byte aligned according to the
; System V ABI standard and de-facto extensions. 
; The compiler will assume the stack is properly aligned 
; and failure to align the stack will result in undefined behavior.
align 16 
stack_space resb 4*1024 ; reserved 4K for kernel stack
kernel_stack_top: 