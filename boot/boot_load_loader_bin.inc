%ifndef _RM_LOAD_LOADER_INC_
%define _RM_LOAD_LOADER_INC_

%include "constants.inc"

; read loader.bin from floppy disk, fat12
boot_load_loader_bin:	

	; reset floppy
	xor ah, ah
	xor dl, dl
	int 0x13

	; search loader.bin in root dir of floppy
	mov word [sector_num], ROOT_DIR_FIRST_SECTOR

.search_loader:
	cmp word [root_dir_sectors], 0 ; loop 14 times max
	jz .no_loader_found
	dec word [root_dir_sectors]
	
	; setup es:bx
	mov ax, LOADER_SEG_BASE
	mov es, ax
	mov bx, LOADER_OFFSET ; es:bx = LOADER_SEG_BASE:LOADER_OFFSET
	
	mov ax, [sector_num] ; ax is the sector_index
	mov cl, 1
	call rm_read_sectors

	mov si, loader_file_name ; ds:si='loader  bin' 11 chars, 2 spaces
	mov di, LOADER_OFFSET    ; es:di=LOADER_SEG_BASE:LOADER_OFFSET
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
	jz .loader_found ; if compard 11 chars are all equal, then found
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
	mov si, loader_file_name
	jmp .search_filename_in_dir_entry
; ---- end loop for search filename

.goto_next_sector_in_root_dir:
	add word [sector_num], 1
	jmp .search_loader

.no_loader_found:
	mov ax, '1'  ; error loader.bin not found
    call boot_putax
	jmp $
.loader_found:
	mov ax, 'A' ; loader found
    call boot_putax	

.begin_load_loader_bin:
	mov ax, ROOT_DIR_SECTORS
	and di, 0xffe0 ; begining of current dir entry
	add di, 0x1a ; di -> first sector
	mov cx, word [es:di]
	push cx ; save index of sector in fat
	add cx, ax
	add cx, DELTA_SECTOR_NUM ; c1 =  first sector of file loader.bin
	mov ax, LOADER_SEG_BASE
	mov es, ax
	mov bx, LOADER_OFFSET
	mov ax, cx

.loop_loading_file:
	
	mov cl, 1
	call rm_read_sectors
	pop ax ; read out the index of the sector in fat
	push bx
	mov bx, LOADER_SEG_BASE
	call rm_get_fat_entry
	pop bx
	cmp ax, 0xfff
	jz .file_loaded

	push ax
	mov dx, ROOT_DIR_SECTORS
	add ax, dx
	add ax, DELTA_SECTOR_NUM
	add bx, [bpb_bytes_per_sector]
	jmp .loop_loading_file
	
.file_loaded:
	mov ax, 'B' ; loader.bin loaded into memory successfully
    call boot_putax
    ; =========================================================
    ; from here, we jump to loader.bin 
    ; and execute the first instruction there.
    ; and also the task of boot sector is completed.

    jmp LOADER_SEG_BASE:LOADER_OFFSET

    ; =========================================================

%include "boot_lib.inc"
%include "rm_read_sectors.inc"
%include "rm_get_fat_entry.inc"

; data section
loader_file_name:       db 'LOADER  BIN', 0    ; 11 chars, 2 spaces, must be UPPER CASE!!
; TODO: use stack variable instead of global variable
sector_num:             dw 0                   ; used to read data from disk
; TODO replace with CONSTANTS
root_dir_sectors:       dw ROOT_DIR_SECTORS    ; used to read data from disk

%endif