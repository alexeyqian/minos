; kernel is first read to at address:0x8000 from disk by loader in real mode 
; then it's been moved to address:0x30000 from 0x8000 to execute in protection mode.
; the reason for read first then move, is because is easy to read from disk in real mode which has BIOS.
; but the kernel is elf format, and cannot run directly as pure bin, since it has format data.
; so it need to parse the elf file and load/move 'segments' into memory, so it can execute. 
; in protected mode, read from disk is hard, but read from memory is easy.
; so read the file data from disk to memory as buffer for disk data, then parse the data in memory and prepare for execution.

; esp, and GDT will also be moved from loader to kernel for easy control

; TODO: move to kernel_entry.inc
KERNEL_SELECTOR equ 8

P_STACKBASE	equ	0              ; process stack base
GSREG		equ	P_STACKBASE
FSREG		equ	GSREG		+ 4
ESREG		equ	FSREG		+ 4
DSREG		equ	ESREG		+ 4
EDIREG		equ	DSREG		+ 4
ESIREG		equ	EDIREG		+ 4
EBPREG		equ	ESIREG		+ 4
KERNELESPREG	equ	EBPREG		+ 4
EBXREG		equ	KERNELESPREG	+ 4
EDXREG		equ	EBXREG		+ 4
ECXREG		equ	EDXREG		+ 4
EAXREG		equ	ECXREG		+ 4
RETADR		equ	EAXREG		+ 4
EIPREG		equ	RETADR		+ 4
CSREG		equ	EIPREG		+ 4
EFLAGSREG	equ	CSREG		+ 4
ESPREG		equ	EFLAGSREG	+ 4
SSREG		equ	ESPREG		+ 4
P_STACKTOP	equ	SSREG		+ 4 ; process stack top
P_LDT_SEL	equ	P_STACKTOP      ; process ldt selector
P_LDT		equ	P_LDT_SEL	+ 4 ; process ldt

TSS3_S_SP0	equ	4

; have to match kernel.h 
SELECTOR_CODE		 equ 0x08		; Defined in loader
SELECTOR_TSS		 equ 0x20		; TSS. jump from outer to inner, the values of SS and ESP will be restored from it.
SELECTOR_KERNEL_CODE equ SELECTOR_CODE

EOI       equ 0x20
INT_M_CTL equ 0x20

; extern global variable
extern gdt_ptr
extern idt_ptr
extern p_proc_ready
extern tss
extern k_reenter
; extern functions
extern kstart
extern kmain
extern exception_handler
extern irq_handler
extern clock_handler
extern kprint
extern delay

[bits 32]

[section .data]
msg_clock_int db "^", 0

[section .bss]
stack_space resb 2*1024
kernel_stack_top: 

[section .text]
global _start
global resume

_start:    	
	mov ebp, kernel_stack_top
	mov esp, ebp

	sgdt [gdt_ptr] ; for moving gdt
	call kstart ; gdt_ptr modified inside kstart
	lgdt [gdt_ptr] ; reload gdt with at new mem location.
	lidt [idt_ptr]

	jmp KERNEL_SELECTOR:csinit

csinit:
	;push 0
	;popfd ; pop top of stack into eflag, set eflags = 0
	;sti ; iretd in IRQ0 will enable it.
	;hlt

	xor eax, eax
	mov ax, SELECTOR_TSS
	ltr ax ; load tss descriptor from GDT to Task Register

	jmp kmain

; ======== not used yet
; general steps to resume a process
; move esp to point to the struct proc pointer
; then pop up all registers to resume the process
resume: 
	mov esp, [p_proc_ready] ; point to the struct proc of ready process
	lldt [esp + P_LDT_SEL] ; equals the p_proc_ready->ldt_sel
	lea eax, [esp + P_STACKTOP] ; point to beginning of eip, cs, eflags, esp, ss
	mov dword [tss + TSS3_S_SP0], eax
resume_krnl_int:
	;dec dword [k_reenter] ; purpose of k_reenter??
	; the pop order have to match the stack_frame inside p_proc_ready
	pop gs ; from top address to bottom: 
	pop fs ; eax, ecx, edx, ebx, esp, ebp, esi edi  (restored by popad)
	pop es ; ds, es, fs, gs.
	pop ds
	popad
	add esp, 4 ; remove retaddr from stack_frame ; PURPOSE OF retaddr?
	iretd      ; so then when do the iretd, the stack is exactly: 
			   ; eip, cs, eflags, esp, ss
			   ; this iretd will reset eflags to proc->regs.eflags
			   ; since we've already set IF before, so after this, IF is set.

; ======== end of not used

; exception and interrupt handlers ==============
; exception handlers
global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error
; interrupt handlers
global	irq00 
global	irq01
global	irq02
global	irq03
global	irq04
global	irq05
global	irq06
global	irq07
global	irq08
global	irq09
global	irq10
global	irq11
global	irq12
global	irq13
global	irq14
global	irq15

; ========== exception handlers ==========
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception
; exception 17 - 31 are intel reserved, not used.

exception:
	call	exception_handler
	add	esp, 4*2	;  make esp point to EIP, remove vec no and error code.
	hlt             

; ========== interrupt handlers ==========
%macro irq_master 1
	push %1
	call irq_handler
	add esp, 4
	hlt
%endmacro

ALIGN	16
irq00:		; Interrupt routine for irq 0 (the clock).
	;irq_master	0
	; when entring interrupt, value of esp is pointing to
	; proc->regs's high addr
	sub esp, 4 ; reserve 4 bytes space: for retaddr

	pushad
	push ds
	push es
	push fs
	push gs

	mov dx, ss
	mov ds, dx
	mov es, dx

	inc byte [gs:0] ; change first cha on screen for testing
	mov al, EOI ; reenable master 8259a
	out INT_M_CTL, al

	inc dword [k_reenter]
	cmp dword [k_reenter], 0
	jne .re_enter

	; IMPORTANT PART
	mov esp, kernel_stack_top ; switch to kernel stack
	sti

	push 0
	call clock_handler
	add esp, 4

	; ... other operations ...
	; testing
	;push msg_clock_int
	;call kprint
	;add esp, 4

	;push 1
	;call delay
	;add esp, 4

	cli

	mov esp, [p_proc_ready] ; leave kernel stack
	lldt [esp + P_LDT_SEL] ; load process LDT

	; setkup tss.esp0 for next process interrupt.
	lea eax, [esp + P_STACKTOP]
	mov dword [tss + TSS3_S_SP0], eax

.re_enter:
	dec dword [k_reenter]
	pop gs
	pop fs
	pop es
	pop ds
	popad

	add esp, 4 ;  remove reserved 4 bytes space

	iretd ; TODO: for now, jump from ring0 to ring1

ALIGN	16
irq01:		; Interrupt routine for irq 1 (keyboard)
	irq_master	1

ALIGN	16
irq02:		; Interrupt routine for irq 2 (cascade!)
	irq_master	2

ALIGN	16
irq03:		; Interrupt routine for irq 3 (second serial)
	irq_master	3

ALIGN	16
irq04:		; Interrupt routine for irq 4 (first serial)
	irq_master	4

ALIGN	16
irq05:		; Interrupt routine for irq 5 (XT winchester)
	irq_master	5

ALIGN	16
irq06:		; Interrupt routine for irq 6 (floppy)
	irq_master	6

ALIGN	16
irq07:		; Interrupt routine for irq 7 (printer)
	irq_master	7

; ---------------------------------
%macro	irq_slave	1
	push	%1
	call	irq_handler
	add	esp, 4
	hlt
%endmacro
; ---------------------------------

ALIGN	16
irq08:		; Interrupt routine for irq 8 (realtime clock).
	irq_slave	8

ALIGN	16
irq09:		; Interrupt routine for irq 9 (irq 2 redirected)
	irq_slave	9

ALIGN	16
irq10:		; Interrupt routine for irq 10
	irq_slave	10

ALIGN	16
irq11:		; Interrupt routine for irq 11
	irq_slave	11

ALIGN	16
irq12:		; Interrupt routine for irq 12
	irq_slave	12

ALIGN	16
irq13:		; Interrupt routine for irq 13 (FPU exception)
	irq_slave	13

ALIGN	16
irq14:		; Interrupt routine for irq 14 (AT winchester)
	irq_slave	14

ALIGN	16
irq15:		; Interrupt routine for irq 15
	irq_slave	15