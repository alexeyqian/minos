; kernel is first read to at address:0x8000 from disk by loader in real mode 
; then it's been moved to address:0x30000 from 0x8000 to execute in protection mode.
; the reason for read first then move, is because is easy to read from disk in real mode which has BIOS.
; but the kernel is elf format, and cannot run directly as pure bin, since it has format data.
; so it need to parse the elf file and load/move 'segments' into memory, so it can execute. 
; in protected mode, read from disk is hard, but read from memory is easy.
; so read the file data from disk to memory as buffer for disk data, then parse the data in memory and prepare for execution.

; esp, and GDT will also be moved from loader to kernel for easy control

kernel_selector equ 8
extern cstart
extern gdt_ptr

[section .bss]
stack_space resb 2*1024
stack_top:

[section .text]
global _start
_start:
    mov	ah, 0xf
	mov	al, 'K'
	mov	[gs:((80 * 10 + 39) * 2)], ax	; row 10 col 39
	
	mov ebp, stack_top
	mov esp, ebp

	sgdt [gdt_ptr] ; store GDTR into memory
	call cstart ; gdt_ptr modified inside cstart
	lgdt [gdt_ptr] ; use new GDT

	jmp kernel_selector:csinit

csinit:
	push 0
	popfd ; pop top of stack into eflag	
	hlt

