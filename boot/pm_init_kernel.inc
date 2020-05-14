pm_init_kernel:
; loop through all program headers, for putting segments into memory
    push ebp
    mov ebp, esp
   
    xor esi, esi
    mov esi, [KERNEL_PHYSICAL_BASE + 0x1c] ; program header offset (header->e_phoff) to esi
    add esi, KERNEL_PHYSICAL_BASE  ; esi = KERNEL_PHYSICAL_BASE + header->e_phoff

    mov cx, word [KERNEL_PHYSICAL_BASE + 0x2c] ;  program header numbers (header->e_phnum) to ecx
    movzx ecx, cx    
.loop:
    mov eax, [esi + 0]
    cmp eax, 0
    jz .no_action

    ; prepare 3 parameters    
    push dword [esi + 0x10] ; param 3: size, 0x10 is offset of field: p_filesz in program header entry
    
    mov eax, [esi + 0x4] ; 0x4 is offset of field: p_offset in PHE
    add eax, KERNEL_PHYSICAL_BASE
    push eax ; param 2: src
    
    push dword [esi + 0x8] ; param 1: dest

    call pm_mem_copy
    add esp, 12 ; clear 3 parameters

.no_action:
    add esi, 0x20 ; esi += header->e_phentsize go to next program header
    dec ecx
    jnz .loop

    mov esp, ebp
    pop ebp
    ret
