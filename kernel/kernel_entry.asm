[bits 32]
[extern kmain] 
;since our boot-strapping code will begin execution blindly 
;from the ﬁrst instruction, so we can make sure that the ﬁrst instruction 
;will eventually result in the kernel’s entry function being reached. 
call kmain
jmp $

%include "utils.inc"
%include "idt_def.inc"
%include "isrs_def.inc"
%include "irq_def.inc"