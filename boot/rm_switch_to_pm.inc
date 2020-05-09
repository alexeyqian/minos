; switch to protected mode
rm_switch_to_pm:
		pusha
	cli
	lgdt [gdt_ptr]

	mov eax, cr0
	or eax, 0x1
	mov cr0, eax

	jmp CODE_SEG:init_pm ; Make a far jump.
	                     ; This also forces the CPU to flush its cache
	                     ; of pre-fetched and real mode decoded instructions, 
	                     ; which can cause problem.

	popa
	ret