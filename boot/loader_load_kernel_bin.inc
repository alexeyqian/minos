%ifndef _RM_LOAD_KERNEL_BIN_
%define _RM_LOAD_KERNEL_BIN_

loader_load_kernel_bin:
    pusha
    
	; reset floppy
    xor ah, ah
    xor dl, dl
    int 0x13

	; search and load kernel from floppy disk
    mov word [sector_num], ROOT_DIR_FIRST_SECTOR
.search_kernel:
	cmp word [root_dir_sectors], 0 ; loop 14 times max
	jz .no_kernel_found
	dec word [root_dir_sectors]
	
	; setup es:bx
	mov ax, KERNEL_BIN_SEG_BASE
	mov es, ax
	mov bx, KERNEL_BIN_OFFSET ; es:bx = KERNEL_BIN_SEG_BASE:KERNEL_BIN_OFFSET
	
	mov ax, [sector_num] ; ax is the sector_index
	mov cl, 1
	call rm_read_sectors

	mov si, kernel_file_name ; ds:si='KERNEL  BIN' 11 chars, 2 spaces
	mov di, KERNEL_BIN_OFFSET    ; es:di=KERNEL_BIN_SEG_BASE:KERNEL_BIN_OFFSET
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
	mov ax, '2' ; kernel.bin not found
    call loader_putax
	jmp $
.kernel_found:
	mov ax, 'E' ; kernel.bin found
    call loader_putax
	
.start_loading_kernel:
	mov ax, ROOT_DIR_SECTORS
	and di, 0xffe0 ; begining of current dir entry

    ; save kernel size
    push eax
    mov eax,  [es:di + 0x1c]
    mov dword [kernel_size], eax
    pop eax

	add di, 0x1a ; di -> first sector
	mov cx, word [es:di]
	push cx ; save index of sector in fat
	add cx, ax
	add cx, DELTA_SECTOR_NUM ; c1 =  first sector of file loader.bin
	mov ax, KERNEL_BIN_SEG_BASE
	mov es, ax
	mov bx, KERNEL_BIN_OFFSET
	mov ax, cx

.loop_loading_file:
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
    mov bx, KERNEL_BIN_SEG_BASE
	call rm_get_fat_entry
    pop bx
	cmp ax, 0xfff
	jz .file_loaded

	push ax
	mov dx, ROOT_DIR_SECTORS
	add ax, dx
	add ax, DELTA_SECTOR_NUM
	add bx, [bpb_bytes_per_sector]
	jc .more_than_64k
	jmp .less_than_64k
.more_than_64k:
	push ax
	mov ax, es
	add ax, 1000h ; es += ox1000, pointing to next segment
	mov es, ax
	pop ax
.less_than_64k:
	jmp .loop_loading_file
.file_loaded:	
    ; stop floppy motor
	mov dx, 0x3f2
    mov al, 0
    out dx, al
	
    popa
    ret
	
; data section
kernel_size:      dd 0 
kernel_file_name: db 'KERNEL  BIN', 0  ; 11 chars, 2 spaces, must be UPPER CASE!!
sector_num:       dw 0                  ; used to read data from disk
root_dir_sectors: dw ROOT_DIR_SECTORS ; used to read data from disk

%endif