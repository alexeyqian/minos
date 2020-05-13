;[section .text]
[bits 32]

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

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
.pm_print_nl:
    push ebp
    mov ebp, esp

    push pm_new_line_str
    call pm_print_str
    add esp, 4

    mov esp, ebp
    pop ebp
    ret

; Prints a null terminated string 
;  string address is pushed as first parameter
pm_print_str_not_used2:
    push ebp
    mov ebp, esp

    push eax
    push ebx
    push edx
    
    mov edx, VIDEO_MEMORY
    mov ebx, [ebp+8] ; assign address of first char in string to ebx   

.loop:
    mov al, [ebx]   ; store char into al
    mov ah, WHITE_ON_BLACK
    cmp al, 0 ; if al == 0 then at end of string
    je .done

    mov [edx], ax ; Store char and attribute at current character cell.
    add ebx, 1 ; make ebx point to next char in the string
    add edx, 2
    jmp .loop

.done:
    pop edx
    pop ebx
    pop eax

    mov esp, ebp
    pop ebp
    ret

; [section .data]
; [bits 32]
; start print on screen position: row 6, col 0
; this variable will be updated along the printing.
pm_position_on_screen: dd (80 * 6 + 0) * 2 

