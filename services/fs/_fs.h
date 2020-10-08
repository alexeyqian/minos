#ifndef _FS_FS_H
#define _FS_FS_H

// POSIX headers
#include <const.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <utils.h>
#include <stdio.h>

// MINOS specific headers
#include <minos/const.h>
#include <minos/types.h>
#include <minos/proto.h>
#include <minos/fs.h>

// TODO: remove hardcode, 
// start sector of inst.tar file
#define	INSTALL_START_SECT		0x8000
#define	INSTALL_NR_SECTS		0x800

/* INODE::i_mode (octal, lower 32 bits reserved) */
#define I_TYPE_MASK     0170000
#define I_REGULAR       0100000
#define I_BLOCK_SPECIAL 0060000
#define I_DIRECTORY     0040000
#define I_CHAR_SPECIAL  0020000
#define I_NAMED_PIPE	0010000

#define	is_special(m)	((((m) & I_TYPE_MASK) == I_BLOCK_SPECIAL) ||	\
			 (((m) & I_TYPE_MASK) == I_CHAR_SPECIAL))

#define	NR_DEFAULT_FILE_SECTS	2048 /* 2048 * 512 = 1MB */


int strip_path(char* filename, const char* path_name, struct inode** ppinode);
int search_file(char* path);
struct inode* get_inode(int dev, int num);
struct super_block* get_super_block(int dev);
int rw_sector(int io_type, int dev, uint64_t pos, int bytes, int proc_nr, void* buf);

int is_valid_inode(struct inode* pin);
void load_super_block(int dev);
int get_dev_driver(int dev);

/**
 * Since all invocations of `rw_sector()' in FS look similar (most of the
 * params are the same), we use this macro to make code more readable.
 *
 * Before I wrote this macro, I found almost every rw_sector invocation
 * line matchs this emacs-style regex:
 * `rw_sector(\([-a-zA-Z0-9_>\ \*()+.]+,\)\{3\}\ *SECTOR_SIZE,\ *TASK_FS,\ *fsbuf)'
 */
// TODO: add param buf to rplace global var: fsbuf
#define RD_SECT(dev,sect_nr) rw_sector(DEV_READ, \
				       dev,				\
				       (sect_nr) * SECTOR_SIZE,		\
				       SECTOR_SIZE, /* read one sector */ \
				       TASK_FS,				\
				       fsbuf);
#define WR_SECT(dev,sect_nr) rw_sector(DEV_WRITE, \
				       dev,				\
				       (sect_nr) * SECTOR_SIZE,		\
				       SECTOR_SIZE, /* write one sector */ \
				       TASK_FS,				\
				       fsbuf);



#endif