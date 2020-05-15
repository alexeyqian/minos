
#include "klib.h"

PUBLIC uint8_t gdt_ptr[6]; // bits 0-15 limits, 16-47 base
PUBLIC struct descriptor gdt[GDT_SIZE];

PUBLIC void kprint_str(char* msg);
PUBLIC void memcpy(void* dest, void* src, int size);

PUBLIC void kstart(){
    kprint_str("\n kstart ...");

    // copy old gdt_ptr to new gdt
    memcpy(&gdt, // new gdt
        (void*)(*((uint32_t*)(&gdt_ptr[2]))),   // Base  of Old GDT
		*((uint16_t*)(&gdt_ptr[0])) + 1	    // Limit of Old GDT
	);

    uint16_t* p_gdt_limit = (uint16_t*)(&gdt_ptr[0]);
	uint32_t* p_gdt_base  = (uint32_t*)(&gdt_ptr[2]);
	*p_gdt_limit = GDT_SIZE * sizeof(struct descriptor) - 1;
	*p_gdt_base  = (uint32_t)&gdt;
}