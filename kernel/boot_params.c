#include "boot_params.h"
#include "types.h"
#include "ktypes.h"
#include "assert.h"
#include "string.h"
#include "elf.h"
#include "screen.h"

// <ring 0>
PUBLIC void print_boot_params(struct boot_params* bp){
	struct mem_range* p;
	for(int i = 0; i < bp->mem_range_count; i++){
		p = &(bp->mem_ranges[i]);
		kprintf("baddr_lo: 0x%x, baddr_hi: 0x%x, len_lo: 0x%x, len_hi: 0x%x, type: %d\n", 
			p->baseaddr_low, p->baseaddr_high, p->length_low, p->length_high, p->type);
	}
}

// <ring 0>
PUBLIC void get_boot_params(struct boot_params* pbp){
	// boot params should have been saved at BOOT_PARAM_ADDR
	uint32_t* p = (uint32_t*)BOOT_PARAM_ADDR;
	kprintf("bp magic: 0x%x at 0x%x\n", p[BI_MAG], BOOT_PARAM_ADDR);	
	assert(p[BI_MAG] == BOOT_PARAM_MAGIC);	
	pbp->kernel_file = (unsigned char *)(p[BI_KERNEL_FILE]);
	kprintf("kernel.bin: 0x%x\n", pbp->kernel_file);		
	assert(memcmp(pbp->kernel_file, ELFMAG, SELFMAG) == 0);
	pbp->mem_size = p[BI_MEM_SIZE];
	kprintf("mem size: 0x%x\n", pbp->mem_size);		
	pbp->mem_range_count = p[BI_MEM_RANGE_COUNT];
	kprintf("mem range count: %d\n", pbp->mem_range_count);		
	memset(pbp->mem_ranges, 0, sizeof(pbp->mem_ranges));
	memcpy(pbp->mem_ranges, &p[BI_MEM_RANGE_BUF], sizeof(pbp->mem_ranges));

	//print_boot_params(pbp);
}

/* 
 * <ring 0> parse the kernel file, get the memory range of the kernel image
 * @param[output] base first valid byte
 * @param[output] limit
 * */
PUBLIC int get_kernel_map(unsigned int* base, unsigned int* limit, struct boot_params* bp){
   
	elf32_ehdr* elf_header = (elf32_ehdr*)(bp->kernel_file);
	if(memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0)
		return -1;

	*base = ~0;
	unsigned int temp = 0;
	for(int i = 0; i < elf_header->e_shnum; i++){
		elf32_shdr* section_header = (elf32_shdr*)(bp->kernel_file + elf_header->e_shoff + i * elf_header->e_shentsize);
		if(section_header->sh_flags & SHF_ALLOC){
			int bottom = section_header->sh_addr;
			int top = section_header->sh_addr + section_header->sh_size;
			if(*base > bottom) *base = bottom;
			if(temp < top) temp = top;
		}
	}

	assert(*base < temp);
	*limit = temp - (*base) - 1;
	kprintf("kernel base: 0x%x, limit: 0x%x\n", *base, *limit);

	return 0;
}