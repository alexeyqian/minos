[bits 32]

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

; ---------------------------------------------------------
; protected mode, print byte in al as hex
; require: 
; - byte in al
; - edi should always points to the next positon on screen for print
; ---------------------------------------------------------
pmprint_byte_in_al_as_hex:
    pusha
    push edi

    mov edi, [_position_on_screen]

    mov ah, WITHE_ON_BLACK

    mov dl, al
    shr al, 4
    mov ecx, 2  ; each byte displayed as two digits, so count set to 2

.begin:
    and al, 0b1111
    cmp al, 9
    ja  .1
    add al, '0'
    jmp .2
.1:
    sub al, 0xa
    add al, 'A'
.2:
    mov gs, VIDEO_MEMORY
    mov [gs:edi], ax
    add edi, 2

    mov al, dl
    loop .begin

    mov [_position_on_screen], edi
    
    pop edi
    popa
    ret

; ---------------------------------------------------------

; ---------------------------------------------------------
; protected mode, print dword pointed by stack as hex
; require: 
; - uint32_t pointed by esp
; - edi should always points to the next positon on screen for print
; ---------------------------------------------------------
pmprint_int_in_stack_as_hex:    
    mov eax, [esp + 4]
    shr eax, 24
    call pmprint_byte_in_al_as_hex

    mov eax, [esp + 4]
    shr eax, 16
    call pmprint_byte_in_al_as_hex

    mov eax, [esp + 4]
    shr eax, 8
    call pmprint_byte_in_al_as_hex

    mov eax, [esp + 4]
    call pmprint_byte_in_al_as_hex

    mov ah, WITHE_ON_BLACK
    mov al, 'h'
    push edi
    push gs
    
    mov edi, [_position_on_screen]
    mov gs, VIDEO_MEMORY
    mov [gs:edi], ax
    add edi, 4
    mov [_position_on_screen], edi
    
    pop gs
    pop edi
   
    ret
; ---------------------------------------------------------


; ---------------------------------------------------------
; protected mode, print string
; ---------------------------------------------------------
pmprint_string:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov esi, [ebp+8]
    mov edi, [_position_on_screen]
    mov ah, WITHE_ON_BLACK
.1:
    lodsb
    test    al, al
    jz  .2
    cmp al, 0xa ; 是回车吗?
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
    mov gs, VIDEO_MEMORY
    mov [gs:edi], ax
    pop gs
    add edi, 2
    jmp .1

.2:
    mov [_position_on_screen], edi

    pop edi
    pop esi
    pop ebx
    pop ebp    
    ret
; ---------------------------------------------------------


; ---------------------------------------------------------
; protected mode, print new line: c: printf("\n");
; ---------------------------------------------------------
pmprint_new_line:
    push _new_line
    call pmprint_string
    ret
; ---------------------------------------------------------

; Prints a null terminated string pointed by EBX
print_string_pm:
    pusha
    mov edx, VIDEO_MEMORY

.loop:
    mov al, [ebx]  ; Store the char at EBX in AL
    mov ah, WHITE_ON_BLACK
    cmp al, 0 ; if al == 0 then at end of string
    je .done

    mov [edx], ax ; Store char and attribute at current character cell.
    add ebx, 1
    add edx, 2
    jmp .loop

.done:
    popa
    ret



; global variables
_position_on_screen:
    dd (80 * 6 + 0) * 2 ; on screen, row 6, col 0
_new_line:
    db 0xa, 0


