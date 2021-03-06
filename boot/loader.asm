; loader: boot loader stage 2
; loader.bin is loaded to 0x90000, 
; loader stack is located at: end of loader.bin in memory + 1K

[bits 16]
%include "constants.inc"
KERNEL_BIN_SEG_BASE     equ 0x7000 
KERNEL_BIN_OFFSET       equ 0x0
KERNEL_BIN_PHYS_ADDR    equ KERNEL_BIN_SEG_BASE * 0x10
KERNEL_PHYS_ENTRY_POINT equ 0x1000 ; must match -Ttext in makefile

; ATTENTION: must match defines in kernel
BOOT_PARAM_MAGIC                equ 0xb007
BOOT_PARAM_ADDR                 equ 0x500
BOOT_PARAM_MEM_RANGE_BUF_SIZE   equ 256

; ================== entry point =================
jmp short start       ; fixed position, start execute from first line of this bin file after loading by boot.
%include "fat12_header.inc" ; contains data used to read kernel.bin from FAT12 floppy to memory
start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
	mov bp, loader_stack_top_rm  
    mov sp, loader_stack_top_rm         

    mov ax, 'C' ; start running in loader
	call loader_putax

	call loader_get_memory_map ; has to be here, otherwise is not working, due to some registers are changes somewhere.
	mov ax, 'D' ; got memory map
    call loader_putax

	; ========== step 1: load kernel.bin file (ELF format) into memory [BIOS] ==========
	call loader_load_kernel_bin
	mov ax, 'F' ; kernel.bin loaded
    call loader_putax		
	
	; ========== step 2: enter protect mode ==========
	lgdt [gdt_ptr]
	cli
	
	in al, 0x92         ; enable A20
	or al, 0b00000010
	out 0x92, al

	mov eax, cr0 	    ; enable protected mode
	or eax, 1
	mov cr0, eax
	; enter protected mode 
	jmp dword code_selector: (LOADER_PHYS_ADDR + pm_start)

%include "rm_read_sectors.inc"
%include "rm_get_fat_entry.inc"
%include "loader_lib.inc"
%include "loader_load_kernel_bin.inc"
%include "loader_descriptor.inc" ; constants and macros
%include "loader_get_memory_map.inc"

; data section
gdt_start:
;                     base-----limit----attr
gdt_null:  Descriptor 0,       0,       0                        
gdt_code:  Descriptor 0,       0xfffff, DA_CR |DA_32|DA_LIMIT_4K  ;  0 - 4G
gdt_data:  Descriptor 0,       0xfffff, DA_DRW|DA_32|DA_LIMIT_4K  ;  0 - 4G
gdt_video: Descriptor 0xb8000, 0xffff,  DA_DRW|DA_DPL3 
gdt_end:

gdt_ptr:
	dw gdt_end - gdt_start - 1            ; gdt length
	dd LOADER_PHYS_ADDR + gdt_start       ; gdt base address

code_selector  equ gdt_code  - gdt_start 
data_selector  equ gdt_data  - gdt_start
video_selector equ gdt_video - gdt_start + SA_RPL3

;=================================================
; bits 32 all below code running in protected mode
;=================================================
[bits 32]
pm_start: ; entry point for protected mode
	mov ax, video_selector
	mov gs, ax
	mov ax, data_selector
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax
	mov ebp, loader_stack_top_pm
	mov esp, ebp

	mov ax, 'G' ; start running in protect mode
    call loader_putax_pm

	call loader_parse_elf_kernel_bin	

	mov ax, 'H' ; kernel loaded
    call loader_putax_pm
	
	call pass_boot_params
	
	; LOADER'S JOB ENDS AFTER THIS JMP
	; ================ enter kernel code ========================
	jmp code_selector: KERNEL_PHYS_ENTRY_POINT
	;=============================================================

%include "loader_lib_pm.inc"
%include "loader_parse_elf_kernel_bin.inc"
%include "loader_pass_boot_params.inc"

; data variables
; 1K appended to the end of loader.bin to be used as stack.
align 32
loader_stack_space: times 1024 db 0 
loader_stack_top_rm    equ   $ ; stack base in real mode
loader_stack_top_pm    equ   LOADER_PHYS_ADDR + $ ; stack base in protected mode
