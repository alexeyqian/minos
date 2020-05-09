; real mode library functions for boot loader
[bits 16]
check_memory_map:
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x100

	mov ebx, 0
	mov di, memory_range_map_buf
.loop:
	mov eax, 0xe820
	mov ecx, 20
	mov edx, 0x534d4150
	int 0x15
	jc check_memory_map_fail

	add di, 20
	inc dword [memory_range_count]
	cmp ebx, 0
	jne .loop
	jmp check_memory_map_ok

check_memory_map_fail:
	mov dword [memory_range_count], 0
check_memory_map_ok:


;global variables
memory_range_count:
	dd 0
memory_range_map_buf:
	times 512 db 0
memory_size:
	dd 0
memory_range_struct:
	base_address_low:   dd 0
	base_address_high:  dd 0
	length_low:         dd 0
	length_high:        dd 0
	type:               dd 0
