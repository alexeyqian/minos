#include "kernel.h"
#include "phys_mem.h"
#include "virt_mem.h"

PRIVATE void init_new_gdt(){
	store_gdt(); // store old gdt to [gdt_ptr] // TODO: use &gdt_ptr to avoid import var to asm

	// copy old gdt table items (not gdt_ptr itself)
	// from loader mem area to new kernel mem area for easy kernel access
    memcpy((char*)&gdt,                         // new gdt table
        (char*)(*((uint32_t*)(&gdt_ptr[2]))),   // base  of old GDT
		*((uint16_t*)(&gdt_ptr[0])) + 1	        // limit of old GDT
	);

	// update the gdt_ptr
	uint32_t* p_gdt_base  = (uint32_t*)(&gdt_ptr[2]);
    uint16_t* p_gdt_limit = (uint16_t*)(&gdt_ptr[0]);
	*p_gdt_base  = (uint32_t)&gdt;
	*p_gdt_limit = GDT_SIZE * sizeof(struct descriptor) - 1;

	load_gdt(); // load new kernel gdt // TODO: use &ddt_ptr to avoid import var to asm
}

#define before_paging_segbase_plus_offset(seg_base, offset) (uint32_t)(((uint32_t)seg_base) + (uint32_t)(offset))
PRIVATE uint32_t before_paging_selector_to_segbase(uint16_t selector){
	struct descriptor* p_dest = &gdt[selector >> 3];
	return (uint32_t)(p_dest->base_high << 24) | (uint32_t)(p_dest->base_mid << 16) | (uint32_t)(p_dest->base_low);
}

PRIVATE void init_tss(){
	memset((char*)&tss, 0, sizeof(tss));
	tss.ss0 = SELECTOR_KERNEL_DATA;
	// setup single tss descriptor in gdt
	init_descriptor((struct descriptor*)&gdt[INDEX_TSS],
			before_paging_segbase_plus_offset(
				before_paging_selector_to_segbase(SELECTOR_KERNEL_DATA), // should = 0x0 
				&tss
			), // TODO: just use &tss should also work here.
			sizeof(tss) - 1,
			DA_386TSS);	
	tss.iobase	= sizeof(tss);	// NO IOPL map

	load_tss(); // TODO: use &tss
}

// each process has one ldt descriptor in gdt.
PRIVATE void init_ldt_descriptors_in_dgt(){
	for(int i = 0; i < NR_TASKS + NR_PROCS; i++){
		memset(&proc_table[i], 0, sizeof(struct proc));

		proc_table[i].ldt_sel = (uint16_t)SELECTOR_LDT_FIRST + (uint16_t)(i << 3);
		kassert(INDEX_LDT_FIRST + i < GDT_SIZE);

		// Fill LDT descriptor in GDT
		init_descriptor(&gdt[INDEX_LDT_FIRST+i],
				before_paging_segbase_plus_offset(
					before_paging_selector_to_segbase(SELECTOR_KERNEL_DATA), // should = 0x0
					proc_table[i].ldt // physical address of LDT
				),
				LDT_SIZE * sizeof(struct descriptor) - 1,
				DA_LDT); 
	}
}

PRIVATE void kmain(){ 
	kprintf(">>> kmain\n");		
	init_proc_table();
	init_clock(); 
	restart(); // pretenting a schedule happend to start a process.
	// process start scheduling at this point.
	while(TRUE){}
}

PUBLIC void kstart(){	
	kprintf(">>> kstart\n");
	init_new_gdt(); 
	init_idt();
	init_tss();
	init_ldt_descriptors_in_dgt(); 
	
	read_boot_params(&g_boot_params); // has to be before init proc table
	pmmgr_init(&g_boot_params);	
	vmmgr_init();
	
	/********** after this line, all addresses become virtual **********/
			
	kmain();
}

