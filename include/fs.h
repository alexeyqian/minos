#ifndef MINOS_FS_H
#define MINOS_FS_H
#include "const.h"
#include "types.h"

// magic number of FS v1.0
#define MAGIC_V1 0x111
// must match boot/include/load.h::SB_MAGIC_V1
#define SUPER_BLK_MAGIC_V1 0x111

// super block, the 2nd sector of the fs
// @attentio: remember to change SUPER_BLOCK_SIZE if the members are changed.
// @attention remember to change boot/include/load.inc::SB_* if members are changed.

struct super_block{
    uint32_t magic; // magic number
    uint32_t nr_inodes;  // max inodes
    uint32_t nr_sects;
    uint32_t nr_imap_sects;
    uint32_t nr_smap_sects; // sector map sectors
    uint32_t n_1st_sect;    // number of the 1st data sector: TODO: rename to first_data_sect

    uint32_t nr_inode_sects; 
    uint32_t root_inode; // inode of root directory
    uint32_t inode_size;
    uint32_t inode_isize_off;  // offset of struct inode::i_size
    uint32_t inode_start_off;  // offset of struct inode::i_start_sect
    uint32_t dir_ent_size;
    uint32_t dir_ent_inode_off; // offset of struct dir_entry::inode_nr
    uint32_t dir_ent_fname_off; // offset of struct dir_entry::name

    // following items are only present in memory
    int sb_dev; // super block's home device
};

// size of struct in the device, not in memory
// size in memory is larger because of some more members
#define SUPER_BLOCK_SIZE 56

// the start_sect and nr_sects locate the file in the device.
// and the size show how many butes in use.
// if size < nr_sect * SECTOR_SIZE, the rest bytes are wasted and reserved for later writing.
// @attention remember to change the INODE_SIZE if members are changed.
struct inode{
    uint32_t i_mode;
    uint32_t i_size; // file size
    uint32_t i_start_sect; // first sector of the data
    uint32_t i_nr_sects;    // how many sectors the file occupies
    uint8_t  _unused[16];  // for alignment

    // only in memory members
    int i_dev;
    int i_cnt; // how many procs share this inode
    int i_num; // node number
};

// size of struct in device not in memory
// size in memory is larger because of some extra memory present only members
#define INODE_SIZE 32

struct file_desc{
    int fd_mode; // R or W
    int fd_pos;  // current position for R/W
    struct inode* fd_inode; // pointer to inode
};


#define MAX_FILENAME_LEN 12
struct dir_entry{
    int inode_nr; 
    char name[MAX_FILENAME_LEN];
};

#define DIR_ENTRY_SIZE sizeof(struct dir_entry)

int rw_sector(int io_type, int dev, uint64_t pos, int bytes, int proc_nr, void* buf);

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

void task_fs();
struct inode* get_inode(int dev, int num);
int search_file(char* path);
int strip_path(char* filename, const char* path_name, struct inode** ppinode);
struct super_block* get_super_block(int dev);
int open(const char* pathname, int flags);
int do_open();
int do_close();
int do_lseek();
#endif