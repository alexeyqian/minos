LOADER_BASE           equ 0x9000 
LOADER_OFFSET         equ 0x100 
LOADER_PHYSICAL_ADDR  equ  LOADER_BASE * 0x10
; the reason for LOADER_PHYSICAL_ADDR is not LOADER_BASE*0x10 + LOADER_OFFSET
; is because the loader is a com/binary file and [org 0x100], 
; which means all addresses in the loader.bin added with 0x100.
; and the loader.bin it self loadded at LOADER_BASE:LOADER_OFFSET in physical memory, 
; so it starts execute at: LOADER_BASE:LOADER_OFFSET;
; and the 0x100 bytes (LOADER_BASE:0 - LOADER_BASE:0x100) 
; are used as stack for loader before enter protected mode.

ROOT_DIR_SECTORS      equ 14  ; 224*32/512
ROOT_DIR_FIRST_SECTOR equ 19
FIRST_SECTOR_OF_FAT1  equ 1
DELTA_SECTOR_NUM      equ 17