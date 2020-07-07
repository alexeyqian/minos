; loader: boot loader stage 2
; loader.bin is loaded to 0x900100, 
; and 0x90000 - 0x900100 is used as stack for loader.bin before entering protected mode.
; after entering PM, the loader using it top 4K space as stack space for itself.
; so means after entering PM, the ebp = esp = 0x90000 + size of loader.bin
; then PM stack grow downward, max size 4K, beyond that will crash the system.
; 4K is way large enough for loader.bin to use.
; after entering kernel, we'll use another stack space.

[org 0x100] 
[bits 16]
%include "constants.inc"

LOADER_STACK_BASE equ 0x100 ;  TODO: move loader stack base to 0x7c00

KERNEL_BASE equ 0x8000 ;  TODO: rename to KERNEL_BIN_BASE, KERNEL_BIN_OFFSET, ...
KERNEL_OFFSET equ 0
KERNEL_PHYSICAL_BASE equ KERNEL_BASE * 0x10
; TODO: rename to KERNEL_RUNTIME_PHYS_ENTRY_POINT
KERNEL_PHYSICAL_ENTRY_POINT equ 0x30400 ; must match -Ttext in makefile
; since ELF has 4K aligned for each segment, so the vaddr will be 0x30000

PAGE_DIR_BASE   equ 0x100000 ; 1M
PAGE_TABLE_BASE equ 0x101000 ; 1M + 4K

; ================== entry point =================
jmp short start       ; fixed position, start execute from first line of this bin file after loading by boot.
%include "fat12_header.inc" ; contains data used to read kernel.bin from FAT12 floppy to memory
start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
	mov bp, LOADER_STACK_BASE  ; data at ss:bp, ss:sp
    mov sp, LOADER_STACK_BASE  ; stack from 0x90100 to 0x90000 (0x100 256 bytes)
       
    mov ax, 'C' ; start running in loader
	call loader_putax

	; ========== step 1: load kernel.bin file (ELF format) into memory [BIOS] ==========
	call loader_load_kernel_bin
	mov ax, 'E' ; kernel.bin loaded
    call loader_putax	
	; ========== step 2: get memory map [BIOS] ==========
	call rm_get_memory_map	
	mov ax, 'F' ; got memory map
    call loader_putax
	; ========== step 3: enter protect mode ==========
	lgdt [gdt_ptr]
	cli
	; enable A20	
	in al, 0x92 ; enable A20
	or al, 0b00000010
	out 0x92, al
	; enable protected mode
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	; jump into protected mode 
	jmp dword code_selector: (LOADER_WITH_STACK_PHYSICAL_ADDR + pm_start)
    ;jmp $ ; should never run to here

%include "rm_read_sectors.inc"
%include "rm_get_fat_entry.inc"
%include "loader_lib.inc"
%include "loader_load_kernel_bin.inc"
%include "loader_descriptor.inc" ; constants and macros
%include "loader_get_memory_map.inc"

; data section
kernel_file_name:       db 'KERNEL  BIN', 0 ; 11 chars, 2 spaces, must be UPPER CASE!!
sector_num:             dw 0 ; used to read data from disk
root_dir_sectors: dw ROOT_DIR_SECTORS  ; used to read data from disk
kernel_size:            dd 0 ; TODO: need to access this data in PM mode

gdt_start:
;                     base-----limit----attr
gdt_null:  Descriptor 0,       0,       0                        
gdt_code:  Descriptor 0,       0xfffff, DA_CR|DA_32|DA_LIMIT_4K  ;  0 - 4G
gdt_data:  Descriptor 0,       0xfffff, DA_DRW|DA_32|DA_LIMIT_4K ;  0 -4G
gdt_video: Descriptor 0xb8000, 0xffff,  DA_DRW|DA_DPL3 
gdt_end:

gdt_ptr:
	dw gdt_end - gdt_start - 1            ; gdt length
	dd LOADER_WITH_STACK_PHYSICAL_ADDR + gdt_start ; gdt base address

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
	mov ebp, pm_loader_stack_top 
	mov esp, ebp

	mov ax, 'G' ; start running in protect mode
    call loader_putax_pm
		
	call pm_print_mem_ranges
	
	jmp $	

	; TODO: move paging setup to kernel c code.
	;call pm_setup_paging
	
	nop
	nop
	nop 

	; TODO: this parse and load elf has to be done after paging enabled???
	call pm_parse_elf_kernel_bin
	jmp $
	; LOADER'S JOB ENDS AFTER THIS JMP
	; ================ enter kernel code ========================
	jmp code_selector: KERNEL_PHYSICAL_ENTRY_POINT
	;=============================================================

%include "loader_lib_pm.inc"
%include "pm_print_mem_ranges.inc"
;%include "pm_setup_paging.inc"
%include "pm_init_kernel.inc"

; data variables
rm_new_line_str: db 0xa, 0
pm_new_line_str equ LOADER_WITH_STACK_PHYSICAL_ADDR + rm_new_line_str

; 1K appended to the end of loader.bin to be used as stack.
align 32
pm_loader_stack_space: times 1024 db 0 
pm_loader_stack_top    equ   LOADER_WITH_STACK_PHYSICAL_ADDR + $ 
