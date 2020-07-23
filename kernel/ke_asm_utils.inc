[bits 32]
[section .text]
; all below functions can only called by c language, follow c language calling convension
; caller clean stack
; TODO: in future, it might be good idea to just use inline asm in C.

global disable_int
disable_int:
	cli
	ret

global enable_int
enable_int:
	sti
	ret

global store_gdt
store_gdt: 
	sgdt [gdt_ptr]  ; global variable
    ret

global load_gdt
load_gdt:
	lgdt [gdt_ptr]  ; global variable
	ret

global load_idt
load_idt:
	lidt [idt_ptr]
	ret

global load_tss
load_tss:
	push eax

	xor eax, eax
	mov ax, SELECTOR_TSS      ; load tss seletor
	ltr ax                    ; which points to a tss descriptor in gdt
	
	pop eax
	ret

global flush_tlb_entry
flush_tlb_entry: ; void flush_tlb(uint32_t addr)        
	cli
	invlpg [esp + 4] ; first parameter addr    
	sti    
	ret

global enable_paging
enable_paging:
    mov	eax, cr0
    or eax, 0x80000000		;set bit 31
	mov	cr0, eax
    ret

global disable_paging
disable_paging:
    mov	eax, cr0
    and eax, 0x7FFFFFFF	
	mov	cr0, eax
    ret

global load_pdbr
load_pdbr: ;void load_pdbr (uint32_t addr);
    mov	eax, [esp + 4]
	mov	cr3, eax
    ret

global get_pdbr
get_pdbr: ;uint32_t get_pdbr();
    mov	eax, cr3
	ret

global get_cr0
get_cr0: ;uint32_t get_cr0();
	mov	eax, cr0
    ret

; u8 in_byte(u16 port)
global in_byte
in_byte:
	mov edx, [esp + 4];
	xor eax, eax
	in al, dx
	nop
	nop
	ret

; void out_byte(u16 port, u8 value);
global out_byte
out_byte:
	mov edx, [esp + 4] ; port
	mov al,  [esp + 8] ; value
	out dx, al
	nop
	nop
	ret

global halt
halt:
    hlt


; ========================================================================
;                  void port_read(u16 port, void* buf, int n);
; ========================================================================
global port_read
port_read:
	mov	edx, [esp + 4]		; port
	mov	edi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1
	cld
	rep	insw
	ret

; ========================================================================
;                  void port_write(u16 port, void* buf, int n);
; ========================================================================
global port_write
port_write:
	mov	edx, [esp + 4]		; port
	mov	esi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1
	cld
	rep	outsw
	ret


; ========================================================================
;                  void disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code for irq < 8:
;       out_byte(INT_CTLMASK, in_byte(INT_CTLMASK) | (1 << irq));
; Returns true if the interrupt was not already disabled.
;
global disable_irq
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
global enable_irq
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
