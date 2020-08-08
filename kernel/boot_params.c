#include "boot_params.h"
#include "types.h"
#include "assert.h"
#include "string.h"
#include "elf.h"
#include "screen.h"

PUBLIC void print_boot_params(struct boot_params* bp){
	for(int i = 0; i < bp->addr_range_count; i++){
		struct addr_range* p = &(bp->addr_ranges[i]);
		printk("baseaddr low: %x, baseaddr high: %x, length low: %x, length high: %x, type: %d\n", 
			p->baseaddr_low, p->baseaddr_high, p->length_low, p->length_high, p->type);
	}
}

PUBLIC void get_boot_params(struct boot_params* pbp){
	// boot params should have been saved at BOOT_PARAM_ADDR
	uint32_t* p = (uint32_t*)BOOT_PARAM_ADDR;
	assert(p[BI_MAG] == BOOT_PARAM_MAGIC);	
	pbp->kernel_file = (unsigned char *)(p[BI_KERNEL_FILE]);
	assert(memcmp(pbp->kernel_file, ELFMAG, SELFMAG) == 0);
	pbp->addr_range_count = p[BI_MEM_RANGE_COUNT];
	memset(pbp->addr_ranges, 0, sizeof(pbp->addr_ranges));
	memcpy(pbp->addr_ranges, p[BI_MEM_RANGE_BUF], pbp->addr_range_count*20);
}

/* 
 * <ring 0-1> parse the kernel file, get the memory range of the kernel image
 * @param[output] base first valid byte
 * @param[output] limit
 * */
PUBLIC int get_kernel_map(unsigned int* base, unsigned int* limit){
    struct boot_params bp;
	get_boot_params(&bp);

	elf32_ehdr* elf_header = (elf32_ehdr*)(bp.kernel_file);
	if(memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0)
		return -1;

	*base = ~0;
	unsigned int temp = 0;
	for(int i = 0; i < elf_header->e_shnum; i++){
		elf32_shdr* section_header = (elf32_shdr*)(bp.kernel_file + elf_header->e_shoff + i * elf_header->e_shentsize);
		if(section_header->sh_flags & SHF_ALLOC){
			int bottom = section_header->sh_addr;
			int top = section_header->sh_addr + section_header->sh_size;
			if(*base > bottom) *base = bottom;
			if(temp < top) temp = top;
		}
	}

	assert(*base < temp);
	*limit = temp - (*base) - 1;

	return 0;
}