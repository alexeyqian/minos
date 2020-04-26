[bits 32]
[extern main] 
;since our boot-strapping code will begin execution blindly 
;from the ﬁrst instruction, so we can make sure that the ﬁrst instruction 
;will eventually result in the kernel’s entry function being reached. 
call main
jmp $