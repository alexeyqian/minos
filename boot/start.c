

PUBLIC t_8 gdt_ptr[6]; // bits 0-15 limits, 16-47 base
PUBLIC DESCRIPTOR gdt[GDT_SIZE];

PUBLIC void print_str(char* msg);
PUBLIC void* memcpy(void* dest, void* src, int size);


PUBLIC void cstart(){
    print_str("\n cstart ...");

    // copy old gdt_ptr to new gdt
    memcpy(&gdt, // new gdt
        (void*)(*((t_32*)(&gdt_ptr[2]))),   // Base  of Old GDT
		*((t_16*)(&gdt_ptr[0])) + 1	    // Limit of Old GDT
	);

    t_16* p_gdt_limit = (t_16*)(&gdt_ptr[0]);
	t_32* p_gdt_base  = (t_32*)(&gdt_ptr[2]);
	*p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
	*p_gdt_base  = (t_32)&gdt;
}