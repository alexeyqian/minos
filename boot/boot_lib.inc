%ifndef _BOOT_LIB_INC_
%define _BOOT_LIB_INC_

;========== function boot_putax: print char in ax at top left =======
boot_putax:
    
    push bx ; save bx
    push es 

    mov ah, 0x0f ; white on black
    mov bx, 0xb800
    mov es, bx
    mov [es:0x0], ax

    pop es
    pop bx ; restore bx
    ret
; ========= end of function =========

%endif