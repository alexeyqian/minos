%ifndef _RM_GET_MEMORY_MAP_INC_
%define _RM_GET_MEMORY_MAP_INC_
[bits 16]

loader_get_memory_map:
    pusha
	; begin get memory range info ========================================
	mov	ebx, 0			            ; ebx = next address range address, init to 0
	mov	di, rm_mem_range_buf		; es:di point to next（Address Range Descriptor Structure）
.mem_check_loop:
	mov	eax, 0E820h		    ; eax = 0000E820h
	mov	ecx, 20			    ; ecx = size of ARD Structure
	mov	edx, 0534D4150h		; edx = 'SMAP' a magic number
	int	15h			
	jc	.mem_check_fail
	add	di, 20
	inc	dword [rm_mem_range_count]
	cmp	ebx, 0
	jne	.mem_check_loop
	jmp	.mem_check_ok
.mem_check_fail:
	mov	dword [rm_mem_range_count], 0
.mem_check_ok:
    popa
    ret

; data section
rm_mem_size:                dd 0
rm_mem_range_count:         dd 0
rm_mem_range_buf:           times BOOT_PARAM_MEM_RANGE_BUF_SIZE db 0 ; buffer to store address range structures array

pm_mem_size        equ LOADER_PHYS_ADDR + rm_mem_size
pm_mem_range_count equ LOADER_PHYS_ADDR + rm_mem_range_count
pm_mem_range_buf   equ LOADER_PHYS_ADDR + rm_mem_range_buf

%endif