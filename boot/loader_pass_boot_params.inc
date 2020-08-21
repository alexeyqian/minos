pass_boot_params:
    push eax
	mov dword [BOOT_PARAM_ADDR], BOOT_PARAM_MAGIC
		
	mov eax, KERNEL_BIN_SEG_BASE
	shl eax, 4
	add eax, KERNEL_BIN_OFFSET
	mov [BOOT_PARAM_ADDR + 4], eax ; physical address of kernel.bin
	
	mov eax, [pm_mem_range_count]
	mov [BOOT_PARAM_ADDR + 8], eax
    pop eax

	; copy mem range boot range buf
.copy_mem_buf:
	push eax
	push ecx
	push esi
	push edi
	
	mov ecx, BOOT_PARAM_MEM_RANGE_BUF_SIZE ; size
	mov esi, pm_mem_range_buf ; src
	mov edi, BOOT_PARAM_ADDR + 12 ; dest
.loop:
	cmp ecx, 0
	jz .done

	mov al, [esi]
	inc esi
	mov byte [edi], al
	inc edi

	dec ecx
	jmp .loop
.done:
	pop edi
	pop esi
	pop ecx
	pop eax

    ret
	