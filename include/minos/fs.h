#ifndef MINOS_FS_H
#define MINOS_FS_H

#include <const.h>
#include <sys/types.h>

#define FD_STDIN  0
#define FD_STDOUT 1
#define FD_STDERR 2

#define	NR_FILES	    64 // max files a proc can open at same time
#define	NR_FILE_DESC	64	/* FIXME */
#define	NR_INODE	    64	/* FIXME */
#define	NR_SUPER_BLOCK	8

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
    uint32_t i_mode;        // TODO: to int
    uint32_t i_size;        // file size TODO: update type to unsigned long
    uint32_t i_start_sect;  // first sector of the data TODO: to int
    uint32_t i_nr_sects;    // how many sectors the file occupies // to int
    uint8_t  _unused[16];   // for alignment

    // only in memory members
    int i_dev;
    int i_cnt;              // how many procs share this inode
    int i_num;              // node number
};

struct file_desc{
    int fd_mode;             // R or W
    int fd_pos;              // current position for R/W
    int fd_cnt;
    struct inode* fd_inode;  // pointer to inode    
};

struct stat{
    int st_dev;  // major/minor device number
    int st_ino;  // i-node number
    int st_mode; // file mode, protection bits, etc
    int st_rdev; // device ID if it's special file
    int st_size; //file size
};

int open(const char* pathname, int flags);
int close(int fd);
int read(int fd, void* buf, int count);
int write(int fd, const void* buf, int count);
int unlink(const char* pathname);
int stat(const char* path, struct stat* buf);

void svc_fs();

#endif