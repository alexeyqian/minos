#ifndef MINOS_FS_H
#define MINOS_FS_H
#include "const.h"
#include "types.h"

/**
 *  super block, the 2nd sector of the fs, sector number = 1
 * 
 * @attention remember to change SUPER_BLOCK_SIZE if the members are changed.
 * @attention remember to change boot/include/load.inc::SB_* if members are changed.
 **/
struct super_block{
    uint32_t magic;            // magic number
    uint32_t nr_inodes;        // max inodes
    uint32_t nr_sects;
    uint32_t nr_imap_sects;
    uint32_t nr_smap_sects;    // sector map sectors
    uint32_t n_1st_sect;       // number of the 1st data sector: TODO: rename to first_data_sect

    uint32_t nr_inode_sects; 
    uint32_t root_inode;       // inode of root directory
    uint32_t inode_size;
    uint32_t inode_isize_off;  // offset of struct inode::i_size
    uint32_t inode_start_off;  // offset of struct inode::i_start_sect
    uint32_t dir_ent_size;
    uint32_t dir_ent_inode_off;// offset of struct dir_entry::inode_nr
    uint32_t dir_ent_fname_off;// offset of struct dir_entry::name

    // following items are only present in memory
    int sb_dev;                // super block's home device
};

/**
 * the start_sect and nr_sects locate the file in the device.
 * and the size show how many butes in use.
 * if size < nr_sect * SECTOR_SIZE, the rest bytes are wasted and reserved for later writing.
 * @attention remember to change the INODE_SIZE if members are changed.
 */ 
struct inode{
    uint32_t i_mode;
    uint32_t i_size;        // file size
    uint32_t i_start_sect;  // first sector of the data
    uint32_t i_nr_sects;    // how many sectors the file occupies
    uint8_t  _unused[16];   // for alignment

    // only in memory members
    int i_dev;
    int i_cnt;              // how many procs share this inode
    int i_num;              // node number
};

struct file_desc{
    int fd_mode;             // R or W
    int fd_pos;              // current position for R/W
    struct inode* fd_inode;  // pointer to inode
};

void task_fs();
int open(const char* pathname, int flags);
int close(int fd);
int read(int fd, void* buf, int count);
int write(int fd, const void* buf, int count);
#endif