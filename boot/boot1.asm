; boot loader stage 1
[org 0x7c00]
[bits 16]

; begin of standard fat boot sector ====================== 
; this format makes dos or windows can recognize the image
jmp boot_start
nop
bs_oem_name:          db 'ForestY' ; must be 8 bytes;
; above 11 bytes will be ignored by fat12

; BIOS Parameter Block
bpb_bytes_per_sector:        dw 512
bpb_sectors_per_clustor:     db 1
bpb_reserved_sector_count:   dw 1
bpb_num_of_fats:             db 2
bpb_root_entity_count:       dw 224
bpb_total_sectors_16:        dw 2880
bpb_media:                   db 0xf0
bpb_sectors_per_fat:         dw 9        
bpb_sectors_per_track:       dw 18       
bpb_num_of_heads:            dw 2        
bpb_hidden_sectors:          dd 0      
bpb_total_sectors_32:        dd 0   ; use this, if bpb_total_sectors_16=0

bs_driver_num:         DB 0        
bs_reserved1:          DB 0        
bs_boot_signature:     DB 29h     
bs_volumn_id:          DD 0        
bs_volumn_label:       DB 'Minos0.01  ' ; must be 11 bytes
bs_file_system_type:   DB 'FAT12   '    ; must be 8 bytes
; end of standard boot sector ======================

RM_STACK_OFFSET equ 0x7bff ; just below 0x7c00, or can use 0x7c00 directly
boot_start:
mov ax, cs
mov ds, ax
mov es, ax
mov ss, ax
mov bp, RM_STACK_OFFSET ; set stack safely away from occupied memory    
mov sp, bp

; BIOS stores our boot drive in dl, 
; so it's best to remember this for later.
mov [boot_drive], dl      

mov bx, msg_start_real_mode
call rm_print_str

jmp $

;call rm_check_mem_map

%include "rm_lib.inc"

; global variables
boot_drive:
    db 0
msg_start_real_mode:
    db 'start in real mode', 0
times 510-($-$$) db 0
dw 0xaa55