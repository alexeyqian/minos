; loader: boot loader stage 2
[org 0x100] ; has to be ox100 ???
[bits 16]
RM_STACK_BASE equ 0x100 ; has to be ox100 ??
KERNEL_BASE equ 0x8000
KERNEL_OFFSET equ 0

ROOT_DIR_SECTORS      equ 14  ; 224*32/512
ROOT_DIR_FIRST_SECTOR equ 19
FIRST_SECTOR_OF_FAT1  equ 1
DELTA_SECTOR_NUM      equ 17

jmp short start
; include it because below code 
; is using some of definitions
%include "fixed_bios_parameter_block.inc"

start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, RM_STACK_BASE  
       
    mov bx, msg_in_loader
    call rm_print_str

    xor ah, ah
    xor dl, dl
    int 0x13

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

    call kill_motor
    jmp $

kill_motor:
    push dx
    mov dx, 0x3f2
    mov al, 0
    out dx, al
    pop dx
    ret

%include "rm_lib.inc"
%include "rm_read_sectors.inc"
%include "rm_get_fat_entry.inc"

; global variables
kernel_file_name:       db 'KERNEL  BIN', 0 ; 11 chars, 2 spaces, must be UPPER CASE!!
msg_in_loader:          db 'in loader|', 0
msg_kernel_found:       db 'kernel found|', 0
msg_no_kernel_found:    db 'kernel not found', 0
msg_kernel_loaded:      db 'kernel loaded|', 0
is_odd:                 db 0
sector_num:             dw 0
kernel_size:            dd 0
root_dir_sectors_for_loop: dw ROOT_DIR_SECTORS