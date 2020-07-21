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