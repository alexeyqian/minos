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
; below defines must match struct stack_frame
P_STACKBASE	equ	0              ; proc->regs stack_base
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

; extern global variable
extern gdt_ptr
extern idt_ptr
extern p_proc_ready
extern tss
extern k_reenter ; reenter for the same interrupt
extern irq_table
; extern functions
extern kinit
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
stack_space resb 2*1024 ; reserved 2K for kernle stack
kernel_stack_top: 

[section .text]
global _start
global restart
global disable_irq
global enable_irq

_start:    	
	;mov ebp, kernel_stack_top
	mov esp, kernel_stack_top ; move esp from loader to kernel

	sgdt [gdt_ptr] ; for moving gdt
	call kinit ; move gdt and init idt, tss, proc_table inside
	lgdt [gdt_ptr] ; reload gdt with at new mem location.
	lidt [idt_ptr]

	jmp KERNEL_SELECTOR:_main

_main:	
	xor eax, eax
	mov ax, SELECTOR_TSS ; load tss seletor
	ltr ax               ; which points to a tss descriptor in gdt

	jmp kmain

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
	call save            ; save all registers into proc table in kernel

	in al, INT_M_CTLMASK ; disable current interrupt
	or al, (1 << %1)
	out INT_M_CTLMASK, al
	
	mov al, EOI          ; reenable master 8259a
	out INT_M_CTL, al
	
	sti                  ; enable interrupt, since CPU disabled interrupt automatcally durint interrupt.
	
	push %1
	call [irq_table + 4 * %1]
	pop ecx
	
	cli                  ; disable interrupt again

	in al, INT_M_CTLMASK ; re-enable current interrupt
	and al, ~(1 << %1)
	out INT_M_CTLMASK, al

	ret                  ; if re_enter, jump to restart_reenter, 
						 ; otherwise go to .restart
%endmacro

ALIGN	16
irq00:		; Interrupt routine for irq 0 (the clock).		
	irq_master 0

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



; ========================= save   =========================
	; jump from ring1/3(user space) into ring0 kernel space.
	; when entring interrupt, the esp/ss is restored from esp0/ss in TSS automatically
	; since we've saved the proc->regs:high address as stack pointer into tss:esp0 before
	; so at beginning esp is point to a high addr of a struct proc->regs in proc_table
	; then CPU automatically (as part of interruption process, invisable to user) 
	; pushes ss->esp->eflags->cs->eip into struct proc(used as stack)
	; value of esp is pointing to just above retaddr
	
	; IT TOOK ME WEEKS TO UNDERSTAND THE STACKS/TSS DURING TASK SWITCH AND REENTER
save:
	;sub esp, 4 ; reserve 4 bytes space: for retaddr	
	pushad     ; save status of current process
	push ds
	push es
	push fs
	push gs    ; at this point, all current process's registers are saved in struct proc->regs

	mov dx, ss
	mov ds, dx
	mov es, dx

	mov eax, esp ; start address of struct proc

	inc dword [k_reenter]
	cmp dword [k_reenter], 0
	jne .reenter             ; if it's reenter, jmp to re_enter
	
	mov esp, kernel_stack_top ; switch to kernel stack
	
	push restart             ; push a return address for not_reenter
	jmp [eax + RETADDR - P_STACKBASE]
.reenter:
	push restart_reenter    ; push a return address for reenter
	jmp [eax + RETADDR - P_STACKBASE]
	
	; DO NOT USE RET HERE

; ========================= restart =========================
restart:
	mov esp, [p_proc_ready] ; leave kernel stack, using next process's struct proc as stack	
	lldt [esp + P_LDT_SEL]  ; load process LDT, prepare for next process

	;====================================================================================
	; TSS3_S_ESP0 is always match proc->regs:high addr, which is correct location for esp
	;====================================================================================
	lea eax, [esp + P_STACKTOP]       ; setup tss.esp0 for p_proc_ready.
	mov dword [tss + TSS3_S_SP0], eax ; TSS3_S_SP0 now is pointing to p_ready_proc->regs high addr
									  ; at the beginning of next interrupt, esp will be copied from tss:esp0 is automatically done by CPU,
									  ; not controlled by programmer.

restart_reenter:                 ; if it's reenter
	dec dword [k_reenter]
	pop gs                            ; restore all registers form p_proc_ready pointed struct proc.
	pop fs
	pop es
	pop ds
	popad
	add esp, 4 ;  remove reserved 4 bytes space

	iretd ; all registers prepared, return and run the next scheduled process

; ==========================================================


; ========================================================================
;                  void disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code for irq < 8:
;       out_byte(INT_CTLMASK, in_byte(INT_CTLMASK) | (1 << irq));
; Returns true if the interrupt was not already disabled.
;
disable_irq:
	mov	ecx, [esp + 4]		; irq
	pushf
	cli
	mov	ah, 1
	rol	ah, cl			; ah = (1 << (irq % 8))
	cmp	cl, 8
	jae	disable_8		; disable irq >= 8 at the slave 8259
disable_0:
	in	al, INT_M_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_M_CTLMASK, al	; set bit at master 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
disable_8:
	in	al, INT_S_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_S_CTLMASK, al	; set bit at slave 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
dis_already:
	popf
	xor	eax, eax		; already disabled
	ret

; ========================================================================
;                  void enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;	}
;
enable_irq:
        mov	ecx, [esp + 4]		; irq
        pushf
        cli
        mov	ah, ~1
        rol	ah, cl			; ah = ~(1 << (irq % 8))
        cmp	cl, 8
        jae	enable_8		; enable irq >= 8 at the slave 8259
enable_0:
        in	al, INT_M_CTLMASK
        and	al, ah
        out	INT_M_CTLMASK, al	; clear bit at master 8259
        popf
        ret
enable_8:
        in	al, INT_S_CTLMASK
        and	al, ah
        out	INT_S_CTLMASK, al	; clear bit at slave 8259
        popf
        ret
