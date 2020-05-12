gdt_start:
;                     base-----limit----attr
gdt_null:  Descriptor 0,       0,       0 ;  LABEL_GDT
gdt_code:  Descriptor 0,       0xfffff, DA_CR|DA_32|DA_LIMIT_4K  ; DESC_FLAT_C 0 - 4G
gdt_data:  Descriptor 0,       0xfffff, DA_DRW|DA_32|DA_LIMIT_4K ; DESC_FLAT_RW 0 -4G
gdt_video: Descriptor 0xB8000, 0xffff,  DA_DRW|DA_DPL3 
gdt_end:

gdt_ptr:
	dw gdt_end - gdt_start - 1            ; gdt length
	dd LOADER_PHYSICAL_ADDR + gdt_start ; gdt base address

CODE_SELECTOR  equ gdt_code  - gdt_start 
DATA_SELECTOR  equ gdt_data  - gdt_start
VIDEO_SELECTOR equ gdt_video - gdt_start + SA_RPL3