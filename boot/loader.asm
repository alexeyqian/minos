; loader: boot loader stage 2
[bits 16]
mov bx, msg_in_loader
call rm_print_str

%include "rm_lib.inc"
; global variables
msg_in_loader: db 'in loader now', 0