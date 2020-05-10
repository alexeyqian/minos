; from sector in ax, read cl sectors into es:bx
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
    ; from here, we get cylinder num, sector num and head number.

    mov dl, [bs_driver_num]
.go_on_reading:
    mov ah, 2   ; read function num
    mov al, byte [bp - 2] ; read al sectors
    int 0x13
    jc .go_on_reading ; if error, cf will be set, it will continue try

    add esp, 2
    pop bp
    ret