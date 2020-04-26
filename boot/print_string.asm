print_string:
    pusha
    mov ah, 0x0e

print_string_loop:
    mov al, [bx]
    cmp al, 0
    je print_string_loop_done
    
    int 0x10
    add bx, 1
    jmp print_string_loop

print_string_loop_done:
    popa
    ret


print_new_line:
    mov ah, 0x0e

    mov al, 0x0d
    int 0x10
    mov al, 0x0a
    int 0x10

    ret