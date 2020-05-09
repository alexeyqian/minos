; real mode, check memory range map
[bits 16]
rm_check_mem_map:
	pusha

	mov ebx, 0
	mov di, mem_range_map
.loop:
	mov eax, 0xe820
	mov ecx, 20
	mov edx, 0x534d4150
	int 0x15
	jc .fail

	add di, 20
	inc dword [mem_range_count]
	cmp ebx, 0
	jne .loop
	jmp .ok

.fail:
	mov dword [mem_range_count], 0
.ok: 
	popa
	ret

;global variables
mem_range_count:
	dd 0
mem_range_map:
	times 512 db 0