%ifndef _RM_READ_SECTORS_INC_
%define _RM_READ_SECTORS_INC_

; used by boot and loader

; from sector index in ax, read cl sectors into es:bx
rm_read_sectors:
    push bp
    mov bp, sp

    sub esp, 2 ; reserve 2 bytes in stack to store
               ; number of sectors. byte [bp - 2]

    mov byte [bp - 2], cl
    push bx
    mov bl, [bpb_sectors_per_track]
    div bl         
    inc ah        
    mov cl, ah  ; start sector       
    mov dh, al          
    shr al, 1          
    mov ch, al  ; cylinder num        
    and dh, 1   ; header num
    pop bx
    ; from here, we get cylinder num, head num and sector num

    mov dl, [bs_driver_num]
.continue_reading:
    mov ah, 2   ; read function num
    mov al, byte [bp - 2] ; read al sectors
    int 0x13
    jc .continue_reading ; keep try when error

    ;add esp, 2 ; clean local variables, no need
    
    mov sp, bp
    pop bp
    ret

%endif