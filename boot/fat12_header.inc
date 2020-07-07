; this file is used by boot.asm and loader.asm

; begin of standard fat boot sector ====================== 
bs_oem_name:                 db 'ForrestY' ; must be 8 bytes;
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
bpb_total_sectors_32:        dd 0            ; use this, if bpb_total_sectors_16=0

bs_driver_num:               DB 0        
bs_reserved1:                DB 0        
bs_boot_signature:           DB 29h     
bs_volumn_id:                DD 0        
bs_volumn_label:             DB 'Minos--0.01' ; must be 11 bytes
bs_file_system_type:         DB 'FAT12   '    ; must be 8 bytes
; end of standard boot sector ======================