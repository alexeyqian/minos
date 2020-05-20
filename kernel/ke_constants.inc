KERNEL_SELECTOR equ 8
; below defines must match struct stack_frame
P_STACKBASE	equ	0               ; proc->regs stack_base
GSREG		equ	P_STACKBASE
FSREG		equ	GSREG		+ 4
ESREG		equ	FSREG		+ 4
DSREG		equ	ESREG		+ 4
EDIREG		equ	DSREG		+ 4
ESIREG		equ	EDIREG		+ 4
EBPREG		equ	ESIREG		+ 4
KESPREG	    equ	EBPREG      + 4 ; kernel esp
EBXREG		equ	KESPREG     + 4
EDXREG		equ	EBXREG		+ 4
ECXREG		equ	EDXREG		+ 4
EAXREG		equ	ECXREG		+ 4
RETADDR		equ	EAXREG		+ 4
EIPREG		equ	RETADDR		+ 4 ;  retrun addr for save()
CSREG		equ	EIPREG		+ 4
EFLAGSREG	equ	CSREG		+ 4
ESPREG		equ	EFLAGSREG	+ 4
SSREG		equ	ESPREG		+ 4

P_STACKTOP	equ	SSREG		+ 4 ; proc.regs stack top
P_LDT_SEL	equ	P_STACKTOP      ; proc->ldt_sel
P_LDT		equ	P_LDT_SEL	+ 4 ; proc->ldt

TSS3_S_SP0	equ	4


; have to match kernel.h 
SELECTOR_CODE		 equ 0x08		; Defined in loader
SELECTOR_TSS		 equ 0x20		; TSS. jump from outer to inner, the values of SS and ESP will be restored from it.
SELECTOR_KERNEL_CODE equ SELECTOR_CODE

EOI             equ 0x20
INT_M_CTL	    equ	0x20	; I/O port for interrupt controller         <Master>
INT_M_CTLMASK	equ	0x21	; setting bits in this port disables ints   <Master>
INT_S_CTL	    equ	0xA0	; I/O port for second interrupt controller  <Slave>
INT_S_CTLMASK	equ	0xA1	; setting bits in this port disables ints   <Slave>