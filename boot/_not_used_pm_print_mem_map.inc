; protected mode, print memory map

---------------------------------------------------------
; protected mode, print memory map
; ---------------------------------------------------------
pmprint_mem_map:
    push esi
    push edi
    push ecx

    mov esi, memory_range_map_buf
    mov ecx, [memory_range_count] ; for(int i = 0; i < mem_range_count; i++)

.loop:
    mov edx, 5 ; for(int j = 0; j < 5; j++) // loop through 5 members
    mov edi, memory_range_struct
.1:
    push dword [esi]
    call pmprint_int_in_stack_as_hex ; print _memory_range_map_buf[j*4] 
    pop eax
    stosd ;  store al at es:di // mem_range_struct[j*4] = mem_range_map_buf[j*4]

    add esi, 4
    dec edx
    cmp edx, 0
    jnz .1
    call pmprint_new_line

    cmp dword [mem_range_type], 1 ; if(type == 1)
    jne .2
    mov eax, [base_address_low]
    add eax, [lengh_low]
    cmp eax, [mem_size] ; if(base_address_low + mem_length_low > mem_size)
    jb .2
    mov [mem_size], eax ; mem_size = base_address_low + mem_length_low
.2:
    loop .loop

    call pmprint_new_line

    push ram_size_str
    call pmprint_string ; printf("ram size");
    add esp, 4

    push dword [mem_size]
    call pmprint_int_in_stack_as_hex
    add esp, 4

    pop ecx
    pop edi
    pop esi
    ret
; ---------------------------------------------------------

; global variables
mem_size:
    dd 0    
mem_range_struct:
    base_address_low:   dd 0
    base_address_high:  dd 0
    length_low:         dd 0
    length_high:        dd 0
    mem_range_type:     dd 0

mem_size_str:
    db 'ram size:', 0
mem_range_table_title:
    db 'addr_low addr_high length_low length_high type', 0
