; load DH sectors to ES:BX from drive DL
disk_load:
    pusha

    push dx       ; Store dx on stack so later we can recall
                  ; how many sectors we requested to be read.
                  ; even if it is altered in the meantime.

    mov ah, 0x02  ; BIOS read sector function
    mov al, dh    ; read dh sectors
    mov ch, 0x00  ; select cylinder 0
    mov dh, 0x00  ; select head 0
    mov cl, 0x02  ; start reading from second sectors
                  ; after the boot sector
    int 0x13      ; BIOS interrupt

    jc disk_error ; jump if error, carry flag set

    pop dx
    cmp dh, al     ; if al (sectors readed) != dh (sectors expected)
    jne disk_error

    popa
    ret

disk_error:
    mov bx, MSG_DIS_ERROR
    call rmprint
    jmp $

; variables
MSG_DIS_ERROR:
    db 'Disk read error!', 0