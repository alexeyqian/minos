[bits 32]

pm_print_mem_ranges:
	push ebp
	mov ebp, esp

	push esi
	push edi
	push ecx

    push pm_mem_table_title
    call pm_print_str	
	add esp, 4 
	call pm_print_nl

	mov esi, pm_mem_range_buf
	mov ecx, [pm_mem_range_count] ; loop through mem range array
.loop_struct:
	mov edx, 5 ; 5 fields in each mem range structures
	mov edi, pm_mem_range_struct
.loop_field: ; second level of loop
    push dword [esi]
    call pm_print_int_as_hex
    pop eax
    stosd ;  mem_range_struct[j*4] = pm_mem_range_buf[j*4]
    add esi, 4
    dec edx
    cmp edx, 0
    jnz .loop_field ; loop for all 5 fields

    call pm_print_nl

    cmp dword [pm_mr_type], 1 ; if memory type is 1
    jne .next_struct
    mov eax, [pm_mr_base_addr_low]
    add eax, [pm_mr_length_low]
    cmp eax, [pm_mem_size] ; if (base_addr_low + length_low > mem_size)
    jb .next_struct
    mov [pm_mem_size], eax ; update mem size to max value
.next_struct:
	loop .loop_struct ; loop through all cx = mem range array items 
.done:	
    call pm_print_nl

    ; print mem size
	push pm_mem_size_str
    call pm_print_str
    add esp, 4
    
    push dword [pm_mem_size]
    call pm_print_int_as_hex
    add esp, 4
    call pm_print_nl
    
	pop ecx
	pop edi
	pop esi

	mov esp, ebp
	pop ebp
	ret