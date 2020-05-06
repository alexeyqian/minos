[bits 16]
SECTORS_TO_READ equ 15

load_kernel_from_disk:
    pusha

    mov bx, MSG_LOAD_KERNEL
    call rmprint
    call rmprint_nl

    ; Set-up parameters for our disk_load routine , so 
    ; that we load the first 15 sectors (excluding 
    ; the boot sector) from the boot disk (i.e. our 
    ; kernel code) to address KERNEL_OFFSET

    push dx       ; Store dx on stack so later we can recall
                  ; how many sectors we requested to be read.
                  ; even if it is altered in the meantime.

    mov bx, KERNEL_OFFSET

    mov al, SECTORS_TO_READ    ; read 15 sectors
    mov ah, 0x02  ; BIOS read sector function
    mov dl, [BOOT_DRIVE] ; read drive 
    mov dh, 0x00  ; select head 0    
    mov ch, 0x00  ; select cylinder 0    
    mov cl, 0x02  ; start reading from second sectors
                  ; after the boot sector
    int 0x13      ; BIOS interrupt

    jc disk_error ; jump if error, carry flag set

    pop dx
    cmp al, SECTORS_TO_READ     ; if al (sectors readed) != dh (sectors expected)
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