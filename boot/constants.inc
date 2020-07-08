%ifndef _CONSTANTS_INC_
%define _CONSTANTS_INC_
; used by boot and loader

ROOT_DIR_SECTORS      equ 14  ; 224*32/512
ROOT_DIR_FIRST_SECTOR equ 19
DELTA_SECTOR_NUM      equ 17

LOADER_SEG_BASE equ 0x9000 
LOADER_OFFSET   equ 0x0 
LOADER_PHYS_ADDR  equ  LOADER_SEG_BASE * 0x10  

%endif