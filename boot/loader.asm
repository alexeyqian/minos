; loader: boot loader stage 2
; loader.bin is loaded to 0x900100, 
; and 0x90000 - 0x900100 is used as stack for loader.bin before entering protected mode.
; after entering PM, the loader using it top 4K space as stack space for itself.
; so means after entering PM, the ebp = esp = 0x90000 + size of loader.bin
; then PM stack grow downward, max size 4K, beyond that will crash the system.
; 4K is way large enough for loader.bin to use.
; after entering kernel, we'll use another stack space.

[org 0x100] 
;[section .text]
[bits 16]
%include "constants.inc"
LOADER_STACK_BASE equ 0x100
KERNEL_BASE equ 0x8000
KERNEL_OFFSET equ 0
KERNEL_PHYSICAL_BASE equ KERNEL_BASE * 0x10
KERNEL_PHYSICAL_ENTRY_POINT equ 0x30400 ; must match -Ttext in makefile

PAGE_DIR_BASE   equ 0x100000 ; 1M
PAGE_TABLE_BASE equ 0x101000 ; 1M + 4K

jmp short start
%include "fixed_bios_parameter_block.inc"
%include "pm.inc"
%include "rm_gdt.inc"

start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, LOADER_STACK_BASE  
       
    mov bx, msg_in_loader
    call rm_print_str

	; begin get memory range info ========================================
	mov	ebx, 0			            ; ebx = next address range address, init to 0
	mov	di, rm_mem_range_buf		; es:di point to next（Address Range Descriptor Structure）
.mem_check_loop:
	mov	eax, 0E820h		    ; eax = 0000E820h
	mov	ecx, 20			    ; ecx = size of ARD Structure
	mov	edx, 0534D4150h		; edx = 'SMAP' a magic number
	int	15h			
	jc	.mem_check_fail
	add	di, 20
	inc	dword [rm_mem_range_count]
	cmp	ebx, 0
	jne	.mem_check_loop
	jmp	.mem_check_ok
.mem_check_fail:
	mov	dword [rm_mem_range_count], 0
.mem_check_ok:
	; end of get memory range info ========================================

	; reset floppy
    xor ah, ah
    xor dl, dl
    int 0x13

	; search and load kernel from floppy disk
    mov word [sector_num], ROOT_DIR_FIRST_SECTOR
.search_kernel:
	cmp word [root_dir_sectors_for_loop], 0 ; loop 14 times max
	jz .no_kernel_found
	dec word [root_dir_sectors_for_loop]
	
	; setup es:bx
	mov ax, KERNEL_BASE
	mov es, ax
	mov bx, KERNEL_OFFSET ; es:bx = KERNEL_BASE:KERNEL_OFFSET
	
	mov ax, [sector_num] ; ax is the sector_index
	mov cl, 1
	call rm_read_sectors

	mov si, kernel_file_name ; ds:si='KERNEL  BIN' 11 chars, 2 spaces
	mov di, KERNEL_OFFSET    ; es:di=KERNEL_BASE:KERNEL_OFFSET
	cld

	; 0x10 = 16 dir entris per sector
	mov dx, 0x10 ; sector size 512 / dir entry size 32 = 16 = 0x10
; ---- loop for search filename
.search_filename_in_dir_entry:
	cmp dx, 0
	jz .goto_next_sector_in_root_dir
	dec dx

	mov cx, 11 ; file name contains 11 chars
.cmp_filename:
	cmp cx, 0
	jz .kernel_found ; if compard 11 chars are all equal, then found
	dec cx	
	lodsb ; ds:si -> al, and increase si at same time
	cmp al, byte [es:di]
	jz .continue_cmp_filename
	jmp .different_filename
.continue_cmp_filename:
	inc di
	jmp .cmp_filename

.different_filename: ; search in next dir entry
	and di, 0xffe0 ; di &= e0, for it to point to start of current entry
	add di, 0x20   ; dir entry size is 0x20, make it point to next dir entry
	mov si, kernel_file_name
	jmp .search_filename_in_dir_entry
; ---- end loop for search filename

.goto_next_sector_in_root_dir:
	add word [sector_num], 1
	jmp .search_kernel

.no_kernel_found:
	mov bx, msg_no_kernel_found
    call rm_print_str
	jmp $
.kernel_found:
	mov bx, msg_kernel_found
    call rm_print_str
	
	mov ax, ROOT_DIR_SECTORS
	and di, 0xffe0 ; begining of current dir entry

    ; save kernel size
    push eax
    mov eax, [es:di + 0x1c]
    mov dword [kernel_size], eax
    pop eax

	add di, 0x1a ; di -> first sector
	mov cx, word [es:di]
	push cx ; save index of sector in fat
	add cx, ax
	add cx, DELTA_SECTOR_NUM ; c1 =  first sector of file loader.bin
	mov ax, KERNEL_BASE
	mov es, ax
	mov bx, KERNEL_OFFSET
	mov ax, cx

.begin_loading_file:
	; print one dot per sector
	;push ax
	;push bx
	;mov ah, 0xe
	;mov al, '.'
	;mov bl, 0xf
	;int 0x10
	;pop bx
	;pop ax

	mov cl, 1
	call rm_read_sectors
	pop ax ; read out the index of the sector in fat
    push bx
    mov bx, KERNEL_BASE
	call rm_get_fat_entry
    pop bx
	cmp ax, 0xfff
	jz .file_loaded

	push ax
	mov dx, ROOT_DIR_SECTORS
	add ax, dx
	add ax, DELTA_SECTOR_NUM
	add bx, [bpb_bytes_per_sector]
	jmp .begin_loading_file
.file_loaded:
	mov bx, msg_kernel_loaded
    call rm_print_str

    ; stop floppy motor
	mov dx, 0x3f2
    mov al, 0
    out dx, al
	
    ; prepare for proteded mode========================
	lgdt [gdt_ptr]
	cli
	in al, 0x92
	or al, 0b00000010
	out 0x92, al

	mov eax, cr0
	or eax, 1
	mov cr0, eax

	; jump into protected mode =============================
	jmp dword code_selector: (LOADER_PHYSICAL_ADDR + pm_start)
    jmp $

%include "rm_lib.inc"
%include "rm_read_sectors.inc"
%include "rm_get_fat_entry.inc"
%include "boot_info.inc"

; [section .data]
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

;=========================================
; bits 32 all below code running in protected mode
;=========================================
;[section .text]
[bits 32]
pm_start:
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

; ================ begin of variables for check and display memory range =====================
rm_mem_size_str: db 'mem size:', 0
rm_mem_size:           dd 0
rm_mem_range_count:    dd 0
rm_mem_range_struct: 
	rm_mr_base_addr_low:  dd 0
	rm_mr_base_addr_high: dd 0
	rm_mr_length_low:     dd 0
	rm_mr_length_high:    dd 0
	rm_mr_type:           dd 0
rm_mem_range_buf: times 512 db 0 ; buffer to store address range structures array
rm_mem_table_title: db  'addr_low addr_high length_low length_high type', 0

pm_mem_size_str equ LOADER_PHYSICAL_ADDR + rm_mem_size_str
pm_mem_size equ LOADER_PHYSICAL_ADDR + rm_mem_size
pm_mem_range_count equ LOADER_PHYSICAL_ADDR + rm_mem_range_count
pm_mem_range_struct equ LOADER_PHYSICAL_ADDR + rm_mem_range_struct
	pm_mr_base_addr_low equ LOADER_PHYSICAL_ADDR + rm_mr_base_addr_low
	pm_mr_base_addr_high equ LOADER_PHYSICAL_ADDR + rm_mr_base_addr_high
	pm_mr_length_low equ LOADER_PHYSICAL_ADDR + rm_mr_length_low
	pm_mr_length_high equ LOADER_PHYSICAL_ADDR + rm_mr_length_high
	pm_mr_type equ LOADER_PHYSICAL_ADDR + rm_mr_type
pm_mem_range_buf equ LOADER_PHYSICAL_ADDR + rm_mem_range_buf
pm_mem_table_title  equ	LOADER_PHYSICAL_ADDR + rm_mem_table_title
; ============= end of variables for check and display memory range =====================

rm_paging_enabled_str: db 'paging is enabled.', 0
pm_paging_enabled_str  equ	LOADER_PHYSICAL_ADDR + rm_paging_enabled_str

; 4K appended to the end of loader.bin to be used as stack.
pm_stack_space: times 0x1000 db 0 
pm_stack_top    equ   LOADER_PHYSICAL_ADDR + $
