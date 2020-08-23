#ifndef MINOS_FS_SHARED_H
#define MINOS_FS_SHARED_H
#include "const.h"
#include "types.h"
#include "fs.h"

// magic number of FS v1.0
#define MAGIC_V1 0x111
// must match boot/include/load.h::SB_MAGIC_V1
#define SUPER_BLK_MAGIC_V1 0x111

// size of struct in the device, not in memory
// size in memory is larger because of some more members
#define SUPER_BLOCK_SIZE 56

// size of struct in device not in memory
// size in memory is larger because of some extra memory present only members
#define INODE_SIZE 32

#define MAX_FILENAME_LEN 12
struct dir_entry{
    int inode_nr; 
    char name[MAX_FILENAME_LEN];
};

#define DIR_ENTRY_SIZE sizeof(struct dir_entry)

int strip_path(char* filename, const char* path_name, struct inode** ppinode);
int search_file(char* path);
struct inode* get_inode(int dev, int num);
struct super_block* get_super_block(int dev);
int rw_sector(int io_type, int dev, uint64_t pos, int bytes, int proc_nr, void* buf);
void reset_inode_table();
void reset_superblock_table();
int is_valid_inode(struct inode* pin);
void load_super_block(int dev);

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