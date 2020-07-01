%ifndef _STDIO_INC_
%define _STDIO_INC_

;************************************************;
;   16 bits functions
;************************************************;

bits 16

; puts16()
; print a null terminated string
; ds:si: 0 terminated string

puts16:
    pusha
.loop:
    lodsb     ; load next byte from string from ds:si to al
    or al, al ; does al = 0?
    jz .done  ; null terminator found, bail out
    mov ah, 0eh
    int 10h
    jmp .loop
.done:
    popa
    ret

;************************************************;
;   32 bits functions
;************************************************;

bits 32
%define VIDMEM 0xb8000
%define COLS   80
%DEFINE LINES  25
%define CHAR_ATTRB 14

_curx db 0 ; current x, y location
_cury db 0

;************************************************;
;putch32()
; prints a char to screen, and move to next location
; bl: char to print

putch32:
    pusha
	mov	edi, VIDMEM
	xor	eax, eax		    ; clear eax

	; y * screen width
	mov	ecx, COLS*2	    	; Mode 7 has 2 bytes per char, so its COLS*2 bytes per line
	mov	al, byte [_cury]	; get y pos
	mul	ecx			        ; multiply y*COLS
	push eax			    ; save eax--the multiplication

	; now add _CurX * 2
	mov	al, byte [_curx]	; multiply _CurX by 2 because it is 2 bytes per char
	mov	cl, 2
	mul	cl
	pop	ecx			        ; pop y*COLS result
	add	eax, ecx

	; add the position to draw to the base of video memory
	xor	ecx, ecx
	add	edi, eax	    	; add it to the base address

	; watch for new line
	cmp	bl, 0x0A		    ; is it a newline character?
	je	.row			    ; yep--go to next row

	; print the character
	mov	dl, bl			    ; Get character
	mov	dh, CHAR_ATTRIB		; the character attribute
	mov	word [edi], dx		; write to video display

	; go to next location

	inc	byte [_curx]		; go to next character
	jmp	.done			    ; nope, bail out

.row:
	mov	byte [_curx], 0		; go back to col 0
	inc	byte [_cury]		; go to next row

.done:
	popa
	ret

;************************************************;
; puts32()
; print a null terminated string
; ebx: address of string to print
puts32:
 pusha
	push	ebx			; copy the string address
	pop	edi

.loop:
	;  Get character
	mov	bl, byte [edi]		; get next character
	cmp	bl, 0			    ; is it 0 (Null terminator)?
	je	.done			    ; yep-bail out
	
	call	putch32			
	
	inc	edi			; go to next character
	jmp	.loop

.done:
	;  update hardware cursor
	mov	bh, byte [_cury]	; get current position
	mov	bl, byte [_curx]
	call	movcur			; update cursor

	popa			
	ret


;************************************************;
; clr32()
; clear screen
clr32:
    pusha
    cld               ; clear direction flag, set DF=0
    mov edi, VIDMEM
    mov cx, 2000
    mov ah, CHAR_ATTRB
    mov al, ' '
    rep stosw         ; store ax at address es:di, rep cx times
    mov byte [_curx], 0
    mov byte [_curx], 0
    popa
    ret

; gotoxy()
; set current xy location
; al = x, ah = y
gotoxy:
    pusha
    mov [_curx], al
    mov [_cury], ah
    popa
    ret
    
;************************************************;
; movecur()
; update hardware cursor
; bl = x pos, bh = y pos
movecur:
    pusha

    ; get current position
    ; loc = _curx + _cury * COLS
    xor eax, eax
    mov ecx, COLS
    mov al, bh     ; get y pos
    mul ecx
    add al, bl
    mov ebx, eax

    ; set low byte index to VGA register
    mov al, 0x0f
    mov dx, 0x03d4    
    out dx, al

    mov al, bl
    mov dx, 0x03d5
    out dx, al 

    ; set high byte index to vga register
    xor	eax, eax

	mov	al, 0x0e
	mov	dx, 0x03D4
	out	dx, al

	mov	al, bh
	mov	dx, 0x03D5
	out	dx, al		

    popa
    ret

%endif