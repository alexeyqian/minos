#ifndef MINOS_HD_H
#define MINOS_HD_H

struct part_info {
	uint32_t	base;	/* # of start sector (NOT byte offset, but SECTOR) */
	uint32_t	size;	/* how many sectors in this partition (NOT byte size, but SECTOR number) */
};

#endif