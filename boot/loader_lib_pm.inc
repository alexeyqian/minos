[bits 32]
VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

;========== function loader_putax: print char in ax at top left =======
loader_putax_pm: 
    mov ah, 0x0f ; white on black
    mov [gs:0x0], ax
    ret

pm_print_al_as_hex:
    push ebp
    mov ebp, esp
	
    push	ecx
	push	edx
	push	edi

	mov	edi, [pm_position_on_screen]

	mov	ah, WHITE_ON_BLACK
	mov	dl, al
	shr	al, 4
	mov	ecx, 2
.loop: ; loop 2 times, display each byte as two hex digitals
	and	al, 0b1111
	cmp	al, 9
	ja	.if_letter
.if_number:
	add	al, '0'
	jmp	.print_char
.if_letter:
	sub	al, 0xa
	add	al, 'A'
.print_char:
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, dl
	loop .loop

.done:
    ;  update pm_position_on_screen to next avialable position
	mov	[pm_position_on_screen], edi 

	pop	edi
	pop	edx
	pop	ecx

    mov esp, ebp
    pop ebp
	ret

pm_print_int_as_hex:
    push ebp
    mov ebp, esp

    mov	eax, [esp + 8]
	shr	eax, 24
	call	pm_print_al_as_hex

	mov	eax, [esp + 8]
	shr	eax, 16
	call	pm_print_al_as_hex

	mov	eax, [esp + 8]
	shr	eax, 8
	call	pm_print_al_as_hex

	mov	eax, [esp + 8]
	call	pm_print_al_as_hex

	mov	ah, WHITE_ON_BLACK
	mov	al, 'h'
	push	edi
	mov	edi, [pm_position_on_screen]
	mov	[gs:edi], ax
	add	edi, 4 ; adding 4 instead of 2 for an extra space after char:h
	mov	[pm_position_on_screen], edi
	pop	edi

    mov esp, ebp
    pop ebp
	ret

; print string in protected mode
pm_print_str:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi

    mov esi, [ebp+8]  ; address of the string
    mov edi, [pm_position_on_screen]
    mov ah, WHITE_ON_BLACK
.loop:
    lodsb ; load byte from ds:si to al
    test    al, al
    jz  .done

    cmp al, 0xa ; is it a new line?
    jnz .if_not_new_line  
 .if_new_line: ; print new line
    ; basically jump locations in video memory
    push    eax
    mov eax, edi
    mov bl, 160
    div bl
    and eax, 0xff
    inc eax
    mov bl, 160
    mul bl
    mov edi, eax
    pop eax

    jmp .loop

.if_not_new_line: ; print char  
    mov [gs:edi], ax    
    add edi, 2
    jmp .loop

.done:
    mov [pm_position_on_screen], edi ; update to next avialable position

    pop edi
    pop esi
    pop ebx

    mov esp, ebp
    pop ebp    
    ret
; ---------------------------------------------------------

; print new line in protected mode 
pm_print_nl:
    push ebp
    mov ebp, esp

    push pm_new_line_str
    call pm_print_str
    add esp, 4

    mov esp, ebp
    pop ebp
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

[bits 32]
; start print on screen position: row 6, col 0
; this variable will be updated along the printing.
pm_position_on_screen: dd (80 * 6 + 0) * 2 

