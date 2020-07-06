%ifndef _RM_GDT_INC_
%define _RM_GDT_INC_

gdt_start:
;                     base-----limit----attr
gdt_null:  Descriptor 0,       0,       0                        
gdt_code:  Descriptor 0,       0xfffff, DA_CR|DA_32|DA_LIMIT_4K  ;  0 - 4G
gdt_data:  Descriptor 0,       0xfffff, DA_DRW|DA_32|DA_LIMIT_4K ;  0 -4G
gdt_video: Descriptor 0xb8000, 0xffff,  DA_DRW|DA_DPL3 
gdt_end:

gdt_ptr:
	dw gdt_end - gdt_start - 1            ; gdt length
	dd LOADER_WITH_STACK_PHYSICAL_ADDR + gdt_start ; gdt base address

code_selector  equ gdt_code  - gdt_start 
data_selector  equ gdt_data  - gdt_start
video_selector equ gdt_video - gdt_start + SA_RPL3

%endif