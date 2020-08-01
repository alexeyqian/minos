#include "fs.h"
#include "./fs_shared.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "string.h"
#include "global.h"
#include "assert.h"
#include "kio.h"
#include "ipc.h"
#include "hd.h"

PUBLIC struct super_block* get_super_block(int dev){
    struct super_block* sb = super_block;
    for(; sb < &super_block[NR_SUPER_BLOCK]; sb++){
        if(sb->sb_dev == dev) return sb;
    }

    panic("super block of device %d not found.\n", dev);
    return 0;
}

/** <ring 1> r/w a sector via messageing 
 * 
 * @param io_type DEV_READ or DEV_WRITE
 * @param dev device number
 * @param pos byte offset from/to where to r/w
 * @param bytes r/w count in bytes
 * @param proc_nr to whom the buffer belongs
 * @param buf r/w buffer
 * 
 * @return zero if success, otherwise panic
 */
PUBLIC int rw_sector(int io_type, int dev, uint64_t pos, int bytes, int proc_nr, void* buf){
    MESSAGE driver_msg;
    driver_msg.type = io_type;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = pos;
    driver_msg.BUF = buf;
    driver_msg.CNT = bytes;
    driver_msg.PROC_NR = proc_nr;
    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);
    return 0;
}


/** 
 * <ring 1> get the inode ptr of given inode number.
 * a cache: inode_table[] maintained to make things faster.
 * if the inode requested is already there, just return it.
 * Otherwise the inode will be read from the disk.
 * 
 * @return inode_ptr requested.
 */
PUBLIC struct inode* get_inode(int dev, int num){
    if(num == 0) return 0;

    struct inode* p;
    struct inode* q = 0;
    for(p = &inode_table[0]; p < &inode_table[NR_INODE]; p++){
        if(p->i_cnt){ // not a free slot
            if((p->i_dev == dev) && (p->i_num == num)){
                // this is the inode we want
                p->i_cnt++;
                return p;
            }
        }else{ // a free slot
            if(!q) q = p; // q hasn't been assigned yet
        }
    }

    if(!q) panic("the inode table is full.");

    // read inode from disk and assgin to the free slot
    q->i_dev = dev;
    q->i_num = num;
    q->i_cnt = 1;

    struct super_block* sb = get_super_block(dev);
    // TODO: wrap below cals to functions for readability
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects +
		((num - 1) / (SECTOR_SIZE / INODE_SIZE));
	RD_SECT(dev, blk_nr);
	struct inode * pinode =
		(struct inode*)((uint8_t*)fsbuf +
				((num - 1 ) % (SECTOR_SIZE / INODE_SIZE))
				 * INODE_SIZE);
	q->i_mode = pinode->i_mode;
	q->i_size = pinode->i_size;
	q->i_start_sect = pinode->i_start_sect;
	q->i_nr_sects = pinode->i_nr_sects;
	return q;
}

/** Search file and return the inode number
 * 
 * @return success: return inode number, faile: return zero
 */
PUBLIC int search_file(char* path){
    uint32_t i, j;

    char filename[MAX_PATH];
    memset(filename, 0, MAX_FILENAME_LEN);
    struct inode* dir_inode;
    if(strip_path(filename, path, &dir_inode) != 0) return 0;

    // path is '/'
    if(filename[0] == 0) return dir_inode->i_num;

    // search the dir for the file
    uint32_t dir_blk0_nr = dir_inode->i_start_sect;
    uint32_t nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE -1)/SECTOR_SIZE;
    uint32_t nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;

    uint32_t m = 0;
    struct dir_entry* pde;
    for(i = 0; i < nr_dir_blks; i++){
        RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
        pde = (struct dir_entry*)fsbuf;
        for(j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++){
            if(memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0)
                return pde->inode_nr;
            if(++m > nr_dir_entries) break;
        }

        // all entries have been iterated
        if(m > nr_dir_entries) break;
    }

    return 0; // file not found
}

/**
 * Get the basename from the fullpath.
 *
 * In FS v1.0, all files are stored in the root directory.
 * There is no sub-folder thing.
 *
 * This routine should be called at the very beginning of file operations
 * such as open(), read() and write(). It accepts the full path and returns
 * two things: the basename and a ptr of the root dir's i-node.
 *
 * e.g. After stip_path(filename, "/blah", ppinode) finishes, we get:
 *      - filename: "blah"
 *      - *ppinode: root_inode
 *      - ret val:  0 (successful)
 *
 * Currently an acceptable pathname should begin with at most one `/'
 * preceding a filename.
 *
 * Filenames may contain any character except '/' and '\\0'.
 *
 * @param[out] filename The string for the result.
 * @param[in]  pathname The full pathname.
 * @param[out] ppinode  The ptr of the dir's inode will be stored here.
 * 
 * @return Zero if success, otherwise the pathname is not valid.
 */
// TODO: re-arrange out params order
PUBLIC int strip_path(char* filename, const char* pathname, struct inode** ppinode){
    const char* s = pathname;
    char* t = filename;

    if(s == 0) return -1;
    if(*s == '/') s++;

    while(*s){
        if(*s == '/') return -1;
        *t++ = *s++;
        // if filename is too long, truncate it
        if(t - filename >= MAX_FILENAME_LEN) break;
    }
    *t = 0;
    *ppinode = root_inode;
    return 0;
}