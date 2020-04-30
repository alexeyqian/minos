[bits 32]
[extern kmain] 
;since our boot-strapping code will begin execution blindly 
;from the ﬁrst instruction, so we can make sure that the ﬁrst instruction 
;will eventually result in the kernel’s entry function being reached. 
call kmain
jmp $

; TODO: below lines should be move to .c file as inline asm
extern idtp
global idt_load
idt_load:
    lidt [idtp]
    ret
    
%include "isrs_def.asm"
%include "irq_def.asm"