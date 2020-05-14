
[section .data]
position_on_screen dd 0

[section .text]
global kprint_str
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
