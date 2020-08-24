#ifndef MINOS_ELF_H
#define MINOS_ELF_H

#include "types.h"

#define EI_NIDENT   16
#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

#define SHF_WRITE		0x1
#define SHF_ALLOC		0x2
#define SHF_EXECINSTR   0x4

#define PT_NULL 0
#define PT_LOAD 1

typedef uint32_t elf32_addr;
typedef uint16_t elf32_half;
typedef uint32_t elf32_off;
typedef int32_t  elf32_sword;
typedef uint32_t elf32_word;

typedef struct{
    unsigned char e_ident[EI_NIDENT];
    elf32_half    e_type;
    elf32_half    e_machine;
    elf32_word    e_version;
    elf32_addr    e_entry;        // entry point
    elf32_off     e_phoff;        // program header table offset
    elf32_off     e_shoff;        // section header table offset
    elf32_word    e_flags;
    elf32_half    e_ehsize;       // elf header size in bytes
    elf32_half    e_phentsize;    // program header entity size
    elf32_half    e_phnum;        // program header number
    elf32_half    e_shentsize;    // section header entity size
    elf32_half    e_shnum;        // section header number
    elf32_half    e_shstrndx;     // index of symble section
} elf32_ehdr;

typedef struct{
    elf32_word p_type;
    elf32_off  p_offset;
    elf32_addr p_vaddr;
    elf32_addr p_paddr;
    elf32_word p_filesz;
    elf32_word p_memsz;
    elf32_word p_flags;
    elf32_word p_align;
} elf32_phdr;

typedef struct{
	elf32_word	sh_name;	    // Section name
	elf32_word	sh_type;	    // Section type.
	elf32_word	sh_flags;	    // Section flags. 
	elf32_addr	sh_addr;	    // Address in memory image. 
	elf32_off	sh_offset;	    // Offset in file.
	elf32_word	sh_size;    	// Size in bytes. 
	elf32_word	sh_link;    	// Index of a related section.
	elf32_word	sh_info;    	// Depends on section type. 
	elf32_word	sh_addralign;	// Alignment in bytes. 
	elf32_word	sh_entsize;	    // Size of each entry in section. 
} elf32_shdr;

#endif