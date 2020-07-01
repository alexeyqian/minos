%ifndef _RM_GET_FAT_ENTRY_INC_
%define _RM_GET_FAT_ENTRY_INC_

; get fat entry
; find index of sector(ax) in fat
; BASE_ADDR is in bx
; return value in ax
rm_get_fat_entry:
	push es
	push bx
	
	push ax
	mov ax, bx ; LOADER_BASE
	sub ax, 0x100 ; reserve 4K for fat behind LOADER_BASE
	mov es, ax
	pop ax

	mov byte [is_odd], 0
	mov bx, 3
	mul bx ; dx:ax = ax*3
	mov bx, 2
	div bx ; dx:ax / 2
	cmp dx, 0
	jz .even
	mov byte [is_odd], 1
.even:
	xor dx, dx ; now the value in ax = index of fat entry in fat
	mov bx, [bpb_bytes_per_sector]
	div bx ; dx:ax / bpb_bytes_per_sector

	push dx
	mov bx, 0
	add ax, FIRST_SECTOR_OF_FAT1 ; after this ax value is the sector num of the fat entry
	mov cl, 2
	call rm_read_sectors ; read 2 sectors, just in case that one fat entry might cross 2 sectors
	pop dx
	add bx, dx
	mov ax, [es:bx]
	cmp byte [is_odd], 1
	jnz .even2
	shr ax, 4
.even2:
	and ax, 0xfff
.get_fat_entry_ok:
	pop bx
	pop es
	ret

%endif