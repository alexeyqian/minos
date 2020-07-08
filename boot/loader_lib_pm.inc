[bits 32]
;========== function loader_putax: print char in ax at top left =======
loader_putax_pm: 
    mov ah, 0x0f ; white on black
    mov [gs:0x0], ax
    ret

; void* mem_copy(void * es:dest, void* ds:src, int size)
pm_mem_copy:
    push ebp
    mov ebp, esp

    push esi
    push edi
    push ecx

    mov edi, [ebp + 8]  ; dest
    mov esi, [ebp + 12] ; src
    mov ecx, [ebp + 16] ; size

.loop:
    cmp ecx, 0
    jz .done

    mov al, [ds:esi];
    inc esi
    mov byte [es:edi], al
    inc edi

    dec ecx
    jmp .loop
    
.done:
    mov eax, [ebp + 8] ; return value in first parameter

    pop ecx
    pop edi
    pop esi

    mov esp, ebp
    pop ebp
    ret