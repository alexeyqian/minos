#include "fs.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "string.h"
#include "global.h"
#include "klib.h"
#include "ipc.h"

/** Decrease the reference number of a slot in inode_table
 * when the number reaches zero, it means the inode is not used 
 * any more and can be overwritten by a new inode.
 * TODO: rename to decrease_inode_count
 * */
PRIVATE void put_inode(struct inode* pinode){
    assert(pinode->i_cnt > 0);
    pinode->i_cnt--;
}

/** <ring 1> write the inode back to the disk
 * commonly invoked as soon as the inode is changed.
 */
PRIVATE void sync_inode(struct inode* p){
    struct inode* pinode;
    struct super_block* sb = get_super_block(p->i_dev);
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects
        + ((p->i_num - 1) / (SECTOR_SIZE / INODE_SIZE));
    RD_SECT(p->i_dev, blk_nr);

    pinode = (struct inode*)((uint8_t*)fsbuf
        + (((p->i_num - 1) % (SECTOR_SIZE / INODE_SIZE)) * INODE_SIZE));

    pinode->i_mode = p->i_mode;
    pinode->i_size = p->i_size;
    pinode->i_start_sect = p->i_start_sect;
    pinode->i_nr_sects = p->i_nr_sects;
    WR_SECT(p->i_dev, blk_nr);
}

// return inode nr
PRIVATE int alloc_imap_bit(int dev){
    int inode_nr = 0;
    int i, j, k;

    int imap_blk0_nr = 1 + 1; // one boot sector & one super block
    struct super_block * sb = get_super_block(dev);

    for(i = 0; i < sb->nr_imap_sects; i++){
        RD_SECT(dev, ima_blk0_nr + i);

        for(j = 0; j < SECTOR_SIZE; j++){
            // skip 11111111 bytes
            if(fsbuf[j] == 0xFF) continue;

            // skip '1' bits
            for(k = 0; ((fsbuf[j] >> k) & 1) != 0; k++){}

            // i: sector index, j: byte index; k: bit index
            inode_nr = (i * SECTOR_SIZE + j) * 8 + k;
            fsbuf[j] |= (1 << k);

            // write the bit to imap
            WR_SECT(dev, impa_blk0_nr + i);
            break;
        }

        return inode_nr;
    }

    panic("inode map is probably full.\n");
    return 0;
}

// return 1st sector nr allocated
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc){
    int i; // sector index
    int j; // byte index
    int k; // bit index

    struct super_block* sb = get_super_block(dev);

    int smap_blk0_nr = 1 + 1 + sb->nr_imap_sects;
    int free_sect_nr = 0;

    for(i = 0; i < sb->nr_smap_sects; i++){
        RD_SECT(dev, smap_blk0_nr + i);

        // byte offset in current sect
        for(j = 0; j < SECTOR_SIZE && nr_sects_to_alloc > 0; j++){
            k = 0;
            if (!free_sect_nr) {
				/* loop until a free bit is found */
				if (fsbuf[j] == 0xFF) continue;
				for (; ((fsbuf[j] >> k) & 1) != 0; k++) {}
				free_sect_nr = (i * SECTOR_SIZE + j) * 8 +
					k - 1 + sb->n_1st_sect;
			}

            for(; k < 8; k++){ // repeat until enough bits are set
                assert(((fsbuf[j] >> k) & 1) == 0);
				fsbuf[j] |= (1 << k);
				if (--nr_sects_to_alloc == 0)
					break;
            }
        }

        if(free_sect_nr) // free bit found, write the bits to smap
            WR_SECT(dev, smap_blk0_nr + i);

        if(nr_sects_to_alloc == 0)
            break;
    }


    assert(nr_sects_to_alloc == 0);
    return free_sect_nr;
}

// genereate a new inode and write it to disk
// return ptr of the new inode
PRIVATE struct inode* new_inode(int dev, int inode_nr, int start_sect){
    struct inode* newinode = get_inode(dev, inode_nr);

    newinode->i_mode = I_REGULAR;
    newinode->i_size = 0;
    newinode->i_start_sect = start_sect;
    newinode->i_nr_sects = NR_DEFAULT_FILE_SECTS;
    newinode->i_dev = dev;
    newinode->i_cnt = 1;
    newinode->i_num = inode_nr;

    // write to the inode array
    sync_inode(newinode);

    return newinode;
}

/**
 * Write a new entry into the directory
 * 
 * @param dir_inode inode of the directory
 * @param inode_nr inode number of the new file
 * @param filename filename of the new file
 * */
PRIVATE void new_dir_entry(struct inode* dir_inode, int inode_nr, char* filename){
    /* write the dir_entry */
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	int nr_dir_entries =
		dir_inode->i_size / DIR_ENTRY_SIZE; /**
						     * including unused slots
						     * (the file has been
						     * deleted but the slot
						     * is still there)
						     */
	int m = 0;
	struct dir_entry * pde;
	struct dir_entry * new_de = 0;

	int i, j;
	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (++m > nr_dir_entries)
				break;

			if (pde->inode_nr == 0) { /* it's a free slot */
				new_de = pde;
				break;
			}
		}
		if (m > nr_dir_entries ||/* all entries have been iterated or */
		    new_de)              /* free slot is found */
			break;
	}
	if (!new_de) { /* reached the end of the dir */
		new_de = pde;
		dir_inode->i_size += DIR_ENTRY_SIZE;
	}
	new_de->inode_nr = inode_nr;
	strcpy(new_de->name, filename);

	/* write dir block -- ROOT dir block */
	WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);

	/* update dir inode */
	sync_inode(dir_inode);

}

/** 
 * create a file and return it's inode ptr
 * @return 0 if failed
 * */
PRIVATE struct inode* create_file(char* path, int flags){
    char filename[MAX_PATH];
    struct inode* dir_inode;

    if(strip_path(filename, path, &dir_inode) != 0)
        return 0;

    int inode_nr = alloc_imap_bit(dir_inode->i_dev);
    int free_sect_nr = alloc_smap_bit(dir_inode->i_dev, NR_DEFAULT_FILE_SECTS);
    struct inode* newino = new_inode(dir_inode->i_dev, inode_nr, free_sect_nr);

    new_dir_entry(dir_inode, newino->i_num, filename);
    return newino;
}

// TODO: refactory to do_open(MESSAGE* msg, struct proc* pcaller)
// open a file and return the file descriptor
PUBLIC int do_open(){
    int fd = -1;
    char pathname[MAX_PATH];

    int flags = fs_msg.FLAGS;
    int name_len = fs_msg.NAME_LEN;
    int src = fs_msg.source; // collar proc number
    assert(name_len < MAX_PATH);
    phys_copy((void*)va2la(TASK_FS, pathname),
        (void*)va2la(src, fs_msg.PATHNAME),
        name_len);
    pathname[name_len] = 0;

    // find a free slot in proc::filp[]
    int i;
    for(i = 0; i < NR_FILES; i++){
        if(pcaller->filp[i] == 0){
            fd = i;
            break;
        }
    }

    if((fd < 0) || (fd >= NR_FILES))
        panic("filp[] is full (pid: %d)", proc2pid(pcaller));

    // find a free slot in f_desc_table[]
    for(i = 0; i < NR_FILE_DESC; i++){
        if(f_desc_table[i].fd_inode == 0)
            break;
    }

    if(i >= NR_FILE_DESC)
        panic("f_desc_table[] is full, pid: %d", proc2pid(pcaller));

    int inode_nr = search_file(pathname);

    struct inode* pin = 0;
    if(flags & O_CREAT){
        if(inode_nr){
            printl("file exists.\n");
            return -1;
        }else{
            pin = create_file(pathname, flags);
        }
    }else{
        assert(flags & O_RDWR);

        char filename[MAX_PATH];
        struct inode* dir_inode;
        if(strip_path(filename, pathname, &dir_inode) != 0)
            return -1;
        pin = get_inode(dir_inode->i_dev, inode_nr);
    }

    if(pin){
        // connects proc with file_descriptor
        pcaller->filp[fd] = &f_desc_table[i];
        // connects file_descriptor with inode
        f_desc_table[i].fd_inode = pin;
        f_desc_table[i].fd_mode = flags;
        // f_desc_table[i].fd_cnt = 1;
        f_desc_table[i].fd_pos = 0;

        int imode = pin->i_mode & I_TYPE_MASK;

        if(imode == I_CHAR_SPECIAL){
            MESSAGE driver_msg;
            driver_msg.type = DEV_OPEN;
            int dev = pin->i_start_sect;
            driver_msg.DEVICE = MINOR(dev);
            assert(MAJOR(dev) == 4);
            assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

            send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);
        }else if(imode == I_DIRECTORY){
            assert(pin->i_num == ROOT_INODE);
        }else{
            assert(pin->i_imode == I_REGULAR);
        }

    }else{
        return -1;
    }

    return fd;
}

// TODO: refactory to do_close(int fd)
PUBLIC int do_close(){
    int fd = fs_msg.FD;
    //TODO: validate fd?

    put_inode(pcaller->filp[fd]->fd_inode); // inode->cnt--
    pcaller->filp[fd]->fd_inode = 0; // disconnect file descriptor from inode
    pcaller->filp[fd] = 0;           // disconnect fd from file descriptors

    return 0;
}

// the new offset in bytes from the beginning of the file if succesfully
// otherwise a negative number.
PUBLIC int do_lseek(){
    int fd = fs_msg.FD;
    int off = fs_msg.OFFSET;
    int whence = fs_msg.WHENCE;

    int pos = pcaller->filp[fd]->fd_pos;
    int f_size = pcaller->filp[fd]->fd_inode->i_size;

    switch(whence){
        case SEEK_SET:
            pos = off;
            break;
        case SEEK_CUR:
            pos += off;
            break;
        case SEEK_END:
            pos = f_size + off; // TODO: ?? + -
            break;
        default:
            return -1;
            break;
    }

    if((pos > f_size) || (pos < 0))
        return -1;

    pcaller->filp[fd]->fd_pos = pos;
    return pos;
}


/** 
 * open/create a file
 * 
 * @param flag O_CREATE, O_RDWR
 * 
 * @return file descriptor if successful, otherwise -1
 */
PUBLIC int open(const char* pathname, int flags){
    MESSAGE msg;
    msg.type = OPEN;
    msg.PATHNAME = (void*)pathname;
    msg.FLAGS = flags;
    msg.NAME_LEN = strlen(pathname);

    send_recv(BOTH, TASK_FS, &msg);
    assert(msg.type == SYSCALL_RET);

    return msg.FD;
}
