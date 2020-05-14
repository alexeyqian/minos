pm_setup_paging:
    push ebp
    mov ebp, esp    

    ; ===== calculate page dir entries  ===================
    ; calculate how many page tables needed based on the physical mem size
    xor edx, edx
    mov eax, [pm_mem_size]
    mov ebx, 0x400000        ; each page table can represents 4M of physical mem.
    div ebx                  ; quotient in eax, and remainder in edx
    mov ecx, eax             ; eax and ecx is the count of page tables, also page dir entry count.
    test edx, edx
    jz .if_no_remainder
.if_remainder:
    inc ecx
.if_no_remainder:
    push ecx ; save page tables count
    ; ===============================================

    ; we simplified the system by making linear address = physical address
    ; and also ignore the mem hole for now.
    
    ; ===== init page dir entries ===================
    mov ax, data_selector;
    mov es, ax
    mov edi, PAGE_DIR_BASE 
    xor eax, eax
    mov eax, PAGE_TABLE_BASE | PG_P | PG_USU | PG_RWW
.loop_add_pde:
    stosd ;  store eax to es:edi
    add eax, 4096
    loop .loop_add_pde
    ; ===============================================

    ; ===== init page tables and their entries ===================
    pop eax ;  get page tables count into eax from stack
    mov ebx, 1024 ; 1024 page table entries
    mul ebx ; multiplies eax and stores the result in edx:eax
    mov ecx, eax ; pte count = page tables * 1024

    mov edi, PAGE_TABLE_BASE
    xor eax, eax
    mov eax, PG_P | PG_USU | PG_RWW
.loop_add_pte:
    stosd
    add eax, 4096 ; each PTE points to 4K mem space
    loop .loop_add_pte
    ; ===============================================

    ; update cr3
    mov eax, PAGE_DIR_BASE
    mov cr3, eax
    ; enable paging in cpu
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    jmp short .done
.done:
    nop   
    
    mov esp, ebp
    pop ebp
    ret
