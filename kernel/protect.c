#include "kernel.h"

PUBLIC void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute)
{
	p_desc->limit_low	     = limit & 0x0FFFF;		         // 段界限 1		(2 字节)
	p_desc->base_low		 = base & 0x0FFFF;		         // 段基址 1		(2 字节)
	p_desc->base_mid		 = (base >> 16) & 0x0FF;		 // 段基址 2		(1 字节)
	p_desc->attr1			 = (uint8_t)(attribute & 0xFF);		     // 属性 1
	p_desc->limit_high_attr2 = (uint8_t)(((limit >> 16) & 0x0F) | ((attribute >> 8) & 0xF0)); // 段界限 2 + 属性 2
	p_desc->base_high		 = (uint8_t)((base >> 24) & 0x0FF);		 // 段基址 3		(1 字节)
}