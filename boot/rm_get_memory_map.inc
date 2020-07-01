%ifndef _RM_GET_MEMORY_MAP_INC_
%define _RM_GET_MEMORY_MAP_INC_
[bits 16]

rm_get_memory_map:
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
    popa
    ret
.mem_check_ok:
    popa
    ret

; data section
; ================ begin of variables for check and display memory range =====================
rm_mem_size_str: db 'mem size:', 0
rm_mem_size:           dd 0
rm_mem_range_count:    dd 0
rm_mem_range_struct: 
	rm_mr_base_addr_low:  dd 0
	rm_mr_base_addr_high: dd 0
	rm_mr_length_low:     dd 0
	rm_mr_length_high:    dd 0
	rm_mr_type:           dd 0
rm_mem_range_buf: times 512 db 0 ; buffer to store address range structures array
rm_mem_table_title: db  'addr_low addr_high length_low length_high type', 0

pm_mem_size_str equ LOADER_PHYSICAL_ADDR + rm_mem_size_str
pm_mem_size equ LOADER_PHYSICAL_ADDR + rm_mem_size
pm_mem_range_count equ LOADER_PHYSICAL_ADDR + rm_mem_range_count
pm_mem_range_struct equ LOADER_PHYSICAL_ADDR + rm_mem_range_struct
	pm_mr_base_addr_low equ LOADER_PHYSICAL_ADDR + rm_mr_base_addr_low
	pm_mr_base_addr_high equ LOADER_PHYSICAL_ADDR + rm_mr_base_addr_high
	pm_mr_length_low equ LOADER_PHYSICAL_ADDR + rm_mr_length_low
	pm_mr_length_high equ LOADER_PHYSICAL_ADDR + rm_mr_length_high
	pm_mr_type equ LOADER_PHYSICAL_ADDR + rm_mr_type
pm_mem_range_buf equ LOADER_PHYSICAL_ADDR + rm_mem_range_buf
pm_mem_table_title  equ	LOADER_PHYSICAL_ADDR + rm_mem_table_title
; ============= end of variables for check and display memory range =====================

%endif