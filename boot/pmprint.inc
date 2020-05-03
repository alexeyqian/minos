[bits 32]

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

; Prints a null terminated string pointed by EBX
print_string_pm:
    pusha
    mov edx, VIDEO_MEMORY

print_string_pm_loop:
    mov al, [ebx]  ; Store the char at EBX in AL
    mov ah, WHITE_ON_BLACK
    cmp al, 0 ; if al == 0 then at end of string
    je print_string_pm_done

    mov [edx], ax ; Store char and attribute at current character cell.
    add ebx, 1
    add edx, 2
    jmp print_string_pm_loop

print_string_pm_done:
    popa
    ret




