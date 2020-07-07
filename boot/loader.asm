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

LOADER_STACK_BASE equ 0x100

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
%include "fixed_bios_parameter_block.inc" ; contains data used to read kernel.bin from FAT12 floppy to memory
start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
	mov bp, LOADER_STACK_BASE  ; data at ss:bp, ss:sp
    mov sp, LOADER_STACK_BASE  ; stack from 0x90100 to 0x90000 (0x100 256 bytes)
       
    mov bx, msg_in_loader
    call rm_print_str

	; ========== step 1: load kernel.bin file (ELF format) into memory [BIOS] ==========
	call rm_load_kernel_bin
	; ========== step 2: get memory map [BIOS] ==========
	call rm_get_memory_map	
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

%include "rm_descriptor.inc" ; constants and macros
%include "rm_gdt.inc" ; data
%include "rm_get_memory_map.inc"
%include "rm_load_kernel_bin.inc"
%include "rm_lib.inc"
%include "rm_read_sectors.inc"
%include "rm_get_fat_entry.inc"

; data section
[bits 16]
kernel_file_name:       db 'KERNEL  BIN', 0 ; 11 chars, 2 spaces, must be UPPER CASE!!
msg_in_loader:          db 'running in loader|', 0
msg_kernel_found:       db 'kernel found|', 0
msg_no_kernel_found:    db 'kernel not found', 0
msg_kernel_loaded:      db 'kernel loaded|', 0
is_odd:                 db 0
sector_num:             dw 0
kernel_size:            dd 0 ; TODO: need to access this data in PM mode
root_dir_sectors_for_loop: dw ROOT_DIR_SECTORS

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

	; TODO clear screen
	
	; TODO: ISSUE HERE
	push pm_running_in_pm_str ; c caller convention, caller prepares parameters.
	call pm_print_str	
	add esp, 4 ; c caller convention, caller cleans parameters
	call pm_print_nl
	
	call pm_print_mem_ranges
	
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

%include "pm_lib.inc"
%include "pm_print_mem_ranges.inc"
;%include "pm_setup_paging.inc"
%include "pm_init_kernel.inc"

; data variables
[bits 32]
rm_new_line_str: db 0xa, 0
rm_running_in_pm_str: db 'running in protected mode now.', 0

pm_new_line_str equ LOADER_WITH_STACK_PHYSICAL_ADDR + rm_new_line_str
pm_running_in_pm_str  equ	LOADER_WITH_STACK_PHYSICAL_ADDR + rm_running_in_pm_str

; 1K appended to the end of loader.bin to be used as stack.
align 32
pm_loader_stack_space: times 1024 db 0 
pm_loader_stack_top    equ   LOADER_WITH_STACK_PHYSICAL_ADDR + $ 
