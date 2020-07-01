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
KERNEL_BASE equ 0x8000
KERNEL_OFFSET equ 0
KERNEL_PHYSICAL_BASE equ KERNEL_BASE * 0x10
KERNEL_PHYSICAL_ENTRY_POINT equ 0x30400 ; must match -Ttext in makefile

PAGE_DIR_BASE   equ 0x100000 ; 1M
PAGE_TABLE_BASE equ 0x101000 ; 1M + 4K

; ================== entry point =================
jmp short start       ; fixed position, start execute from first line of this bin file after loading by boot.

%include "fixed_bios_parameter_block.inc" ; data
%include "pm.inc" ; constants and macros
%include "multiboot_info.inc"
%include "rm_gdt.inc" ; data

start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, LOADER_STACK_BASE  ; stack from 0x9000:0x100 to 0x9000:0x0 (0x100 256 bytes)
       
    mov bx, msg_in_loader
    call rm_print_str
	
	call rm_load_kernel_bin

    ; prepare for proteded mode
	
	lgdt [gdt_ptr]
	
	; enable A20
	cli
	in al, 0x92 ; enable A20
	or al, 0b00000010
	out 0x92, al

	; after enable A20, use bios to read memory size and map
	; before A20 is enabled, system cannot get memory over 1M
	call rm_get_memory_map
	; TODO: fill kernel info struct to pass as parameters to kernel
	; TODO: call rm_fill_boot_info

	mov eax, cr0
	or eax, 1
	mov cr0, eax

	; jump into protected mode 
	jmp dword code_selector: (LOADER_PHYSICAL_ADDR + pm_start)
    jmp $

%include "rm_load_kernel_bin.inc"
%include "rm_get_memory_map.inc"
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
kernel_size:            dd 0
root_dir_sectors_for_loop: dw ROOT_DIR_SECTORS

boot_info:
istruc multiboot_info
	at multiboot_info.flags,			dd 0
	at multiboot_info.memoryLo,			dd 0
	at multiboot_info.memoryHi,			dd 0
	at multiboot_info.bootDevice,		dd 0
	at multiboot_info.cmdLine,			dd 0
	at multiboot_info.mods_count,		dd 0
	at multiboot_info.mods_addr,		dd 0
	at multiboot_info.syms0,			dd 0
	at multiboot_info.syms1,			dd 0
	at multiboot_info.syms2,			dd 0
	at multiboot_info.mmap_length,		dd 0
	at multiboot_info.mmap_addr,		dd 0
	at multiboot_info.drives_length,	dd 0
	at multiboot_info.drives_addr,		dd 0
	at multiboot_info.config_table,		dd 0
	at multiboot_info.bootloader_name,	dd 0
	at multiboot_info.apm_table,		dd 0
	at multiboot_info.vbe_control_info,	dd 0
	at multiboot_info.vbe_mode_info,	dw 0
	at multiboot_info.vbe_interface_seg,dw 0
	at multiboot_info.vbe_interface_off,dw 0
	at multiboot_info.vbe_interface_len,dw 0
iend

;=================================================
; bits 32 all below code running in protected mode
;=================================================
;[section .text]
[bits 32]
pm_start: ; entry point for protected mode
	mov ax, video_selector
	mov gs, ax
	mov ax, data_selector
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax
	mov ebp, pm_stack_top
	mov esp, ebp

	; TODO clear screen

	push pm_running_in_pm_str ; c caller convention, caller prepares parameters.
	call pm_print_str	
	add esp, 4 ; c caller convention, caller cleans parameters
	call pm_print_nl

	call pm_print_mem_ranges	
	
	call pm_setup_paging

	push pm_paging_enabled_str
	call pm_print_str	
	add esp, 4
	call pm_print_nl

	call pm_init_kernel

	; LOADER'S JOB ENDS AFTER THIS JMP
	; ================ enter kernel code ========================
	jmp code_selector: KERNEL_PHYSICAL_ENTRY_POINT
	;=============================================================
	
	;jmp $

%include "pm_lib.inc"
%include "pm_print_mem_ranges.inc"
%include "pm_setup_paging.inc"
%include "pm_init_kernel.inc"

;[section .data]
[bits 32]
rm_new_line_str: db 0xa, 0
rm_running_in_pm_str: db 'running in protected mode now.', 0

pm_new_line_str equ LOADER_PHYSICAL_ADDR + rm_new_line_str
pm_running_in_pm_str  equ	LOADER_PHYSICAL_ADDR + rm_running_in_pm_str

rm_paging_enabled_str: db 'paging is enabled.', 0
pm_paging_enabled_str  equ	LOADER_PHYSICAL_ADDR + rm_paging_enabled_str

; 4K appended to the end of loader.bin to be used as stack.
pm_stack_space: times 0x1000 db 0 
pm_stack_top    equ   LOADER_PHYSICAL_ADDR + $
