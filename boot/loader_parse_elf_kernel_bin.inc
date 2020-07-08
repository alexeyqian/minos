[bits 32]
; loop through all program headers 
; and putting kernel segments into memory
loader_parse_elf_kernel_bin:
    push ebp
    mov ebp, esp       

    xor esi, esi
    mov esi, [KERNEL_BIN_PHYS_ADDR + 0x1c]     ; program header offset (header->e_phoff) to esi
    add esi, KERNEL_BIN_PHYS_ADDR              ; esi = KERNEL_BIN_PHYS_ADDR + header->e_phoff

    mov cx, word [KERNEL_BIN_PHYS_ADDR + 0x2c] ;  program header numbers (header->e_phnum) to ecx
    movzx ecx, cx    
.loop:
    mov eax, [esi + 0]
    cmp eax, 0
    jz .no_action ; if the segment type is 0, then ignore it

    ;mov ax, data_selector ; setup es and ds for memcpy
    ;mov ds, ax
    ;mov es, ax   
    
    ; TODO: ?? use filesz instead of memsz? might cause problems for .bss section.
    ; KERNLE should not use bss section?
    ; prepare 3 parameters    
    push dword [esi + 0x10] ; param 3: [size], 0x10 is offset of field: p_filesz in program header entry
    
    mov eax, [esi + 0x4] ; 0x4 is [offset] of field: p_offset in PHE
    add eax, KERNEL_BIN_PHYS_ADDR
    push eax ; param 2: src
    
    push dword [esi + 0x8] ; 0x8 is [vaddr=0x30000], param 1: dest ; TODO: dest using vaddr, need paging enabled, not good

    call pm_mem_copy
    add esp, 12 ; clear 3 parameters

.no_action:
    add esi, 0x20 ; esi += header->e_phentsize go to next program header
    dec ecx
    jnz .loop

.done:
    mov esp, ebp
    pop ebp
    ret
