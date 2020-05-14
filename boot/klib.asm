
WHITE_ON_BLACK equ 0x0f

[section .data]
position_on_screen dd 0

[section .text]
global kprint_str
global memcpy

; print string in protected mode, void kprint_str(char* msg)
kprint_str:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi

    mov esi, [ebp+8]  ; address of the string
    mov edi, [position_on_screen]
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
    mov [position_on_screen], edi ; update to next avialable position

    pop edi
    pop esi
    pop ebx

    mov esp, ebp
    pop ebp    
    ret
; ---------------------------------------------------------


; void* memcpy(void * es:dest, void* ds:src, int size)
memcpy:
    push ebp
    mov ebp, esp

    push esi
    push edi
    push ecx

    mov edi, [ebp + 8] ;dest
    mov esi, [ebp + 12] ; src
    mov ecx, [ebp + 16] ; counter

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
; ---------------------------------------------------------