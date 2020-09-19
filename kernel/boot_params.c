#include "boot_params.h"
#include "types.h"
#include "ktypes.h"

#include "string.h"
#include "elf.h"
#include "screen.h"

// @attention must match code in loader.asm
#define BOOT_PARAM_ADDR  0x500
#define BOOT_PARAM_MAGIC 0xb007
#define	BI_MAG			     0
#define	BI_KERNEL_FILE	     1
#define	BI_MEM_RANGE_COUNT	 2
#define	BI_MEM_RANGE_BUF	 3

PRIVATE void print_mem_ranges(struct boot_params* bp){
	struct mem_range* p;
	for(uint32_t i = 0; i < bp->mem_range_count; i++){
		p = &(bp->mem_ranges[i]);
		kprintf("baddr_lo: 0x%x, baddr_hi: 0x%x, len_lo: 0x%x, len_hi: 0x%x, type: %d\n", 
			p->baseaddr_low, p->baseaddr_high, p->length_low, p->length_high, p->type);
	}
}

PRIVATE void set_mem_size(struct boot_params* bp){
	struct mem_range* p;
	uint32_t mem_size = 0;
	uint32_t temp_mem_size = 0;
	for(uint32_t i = 0; i < bp->mem_range_count; i++){
		p = &(bp->mem_ranges[i]);
		if(p->type != ADDR_RANGE_MEMORY) continue;

		temp_mem_size = p->baseaddr_low + p->length_low;
		if(temp_mem_size > mem_size)
			mem_size = temp_mem_size;		
	}
	bp->mem_size = mem_size;
}

// parse the kernel.bin file, get the memory range of the kernel image
PRIVATE void set_kernel_base_limit(struct boot_params* bp){   
	elf32_ehdr* elf_header = (elf32_ehdr*)(bp->kernel_file);
	if(memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0)
		return;

	bp->kernel_base = ~((uint32_t)0);
	uint32_t temp = 0;
	for(uint32_t i = 0; i < elf_header->e_shnum; i++){
		elf32_shdr* section_header = (elf32_shdr*)(bp->kernel_file + elf_header->e_shoff + i * elf_header->e_shentsize);
		if(section_header->sh_flags & SHF_ALLOC){
			uint32_t bottom = section_header->sh_addr;
			uint32_t top = section_header->sh_addr + section_header->sh_size;
			if(bp->kernel_base > bottom) bp->kernel_base = bottom;
			if(temp < top) temp = top;
		}
	}

	kassert(bp->kernel_base < temp);
	bp->kernel_limit = temp - bp->kernel_base - 1;
	//kprintf("kernel base: 0x%x, limit: 0x%x\n", bp->kernel_base, bp->kernel_limit);
}

PUBLIC void get_boot_params(struct boot_params* pbp){
	// boot params should have been saved at BOOT_PARAM_ADDR
	uint32_t* p = (uint32_t*)BOOT_PARAM_ADDR;
	kprintf("bp magic: 0x%x at 0x%x\n", p[BI_MAG], BOOT_PARAM_ADDR);	
	kassert(p[BI_MAG] == BOOT_PARAM_MAGIC);	

	pbp->kernel_file = (unsigned char *)(p[BI_KERNEL_FILE]);
	kprintf("kernel.bin: 0x%x\n", pbp->kernel_file);		
	kassert(memcmp(pbp->kernel_file, ELFMAG, SELFMAG) == 0);

	// mem range count
	pbp->mem_range_count = p[BI_MEM_RANGE_COUNT];
	kprintf("mem range count: %d\n", pbp->mem_range_count);		

	// mem range
	memset(pbp->mem_ranges, 0, sizeof(pbp->mem_ranges));
	memcpy(pbp->mem_ranges, &p[BI_MEM_RANGE_BUF], sizeof(pbp->mem_ranges));
	print_mem_ranges(pbp);
	
	// mem size, calculated from mem ranges
	set_mem_size(pbp);	
	kprintf("mem size: 0x%x\n", pbp->mem_size);			
	
	// kernel base and limit
	set_kernel_base_limit(pbp);
}