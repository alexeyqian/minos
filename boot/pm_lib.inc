[bits 32]

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f
; ---------------------------------------------------------
; protected mode, print string
; ---------------------------------------------------------
pm_print_str:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi

    mov esi, [ebp+8]  ; address of the string
    mov edi, [position_on_screen]
    mov ah, WHITE_ON_BLACK
.1:
    lodsb
    test    al, al
    jz  .2
    cmp al, 0xa ; is it a return
    jnz .3
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
    jmp .1
.3:
    push gs
    mov ebx, VIDEO_MEMORY
    mov gs, ebx
    mov [gs:edi], ax
    pop gs
    add edi, 2
    jmp .1

.2:
    mov [position_on_screen], edi

    pop edi
    pop esi
    pop ebx

    mov esp, ebp
    pop ebp    
    ret
; ---------------------------------------------------------

; global variables
position_on_screen:
    dd (80 * 6 + 0) * 2 ; on screen, row 6, col 0

