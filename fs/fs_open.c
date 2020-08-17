#include "fs.h"
#include "./fs_const.h"
#include "./fs_open.h"
#include "./fs_shared.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "string.h"
#include "global.h"
#include "klib.h"
#include "kio.h"
#include "assert.h"
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
    uint32_t i, j, k;

    uint32_t imap_blk0_nr = 1 + 1; // one boot sector & one super block
    struct super_block * sb = get_super_block(dev);

    for(i = 0; i < sb->nr_imap_sects; i++){
        RD_SECT(dev, imap_blk0_nr + i);

        for(j = 0; j < SECTOR_SIZE; j++){
            // skip 11111111 bytes
            if(fsbuf[j] == 0xFF) continue;

            // skip '1' bits
            for(k = 0; ((fsbuf[j] >> k) & 1) != 0; k++){}

            // i: sector index, j: byte index; k: bit index
            inode_nr = (i * SECTOR_SIZE + j) * 8 + k;
            fsbuf[j] |= (1 << k);

            // write the bit to imap
            WR_SECT(dev, imap_blk0_nr + i);
            break;
        }

        return inode_nr;
    }

    panic("inode map is probably full.\n");
    return 0;
}

// return 1st sector nr allocated
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc){
    uint32_t i; // sector index
    uint32_t j; // byte index
    uint32_t k; // bit index

    struct super_block* sb = get_super_block(dev);

    uint32_t smap_blk0_nr = 1 + 1 + sb->nr_imap_sects;
    uint32_t free_sect_nr = 0;

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
	uint32_t dir_blk0_nr = dir_inode->i_start_sect;
	uint32_t nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	uint32_t nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE; 
	uint32_t m = 0;
	struct dir_entry * pde;
	struct dir_entry * new_de = 0;
	uint32_t i, j;
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
    kprintf("here 6.5");

}

/** 
 * create a file and return it's inode ptr
 * @return 0 if failed
 * */
PRIVATE struct inode* create_file(char* path, int flags){
    UNUSED(flags);

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

    if((fd < 0) || (fd >= NR_FILES)){
        panic("filp[] is full (pid: %d)", proc2pid(pcaller));
    }
        

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
            kprintf("file exists.\n");
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
        f_desc_table[i].fd_cnt = 1;
        f_desc_table[i].fd_pos = 0;

        int imode = pin->i_mode & I_TYPE_MASK;
        if(imode == I_CHAR_SPECIAL){
            MESSAGE driver_msg;
            driver_msg.type = DEV_OPEN;
            int dev = pin->i_start_sect;
            driver_msg.DEVICE = MINOR(dev);
            assert(MAJOR(dev) == 4);
            assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
            kprintf(">>> 3.3 in do_open(), before send, src: %d, type: %d\n", dd_map[MAJOR(dev)].driver_nr, driver_msg.type);
            send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);
            kprintf(">>> 3.3 in do_open(), after send, src: %d, type: %d\n", dd_map[MAJOR(dev)].driver_nr, driver_msg.type);
        }else if(imode == I_DIRECTORY){
            assert(pin->i_num == ROOT_INODE);
        }else{
            assert(pin->i_mode == I_REGULAR);
        }

    }else{
        return -1;
    }

    return fd;
}

// TODO: refactory to do_close(int fd)
PUBLIC int do_close(){
    int fd = fs_msg.FD;
    put_inode(pcaller->filp[fd]->fd_inode); // inode->cnt--
    if(--pcaller->filp[fd]->fd_cnt == 0)
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
 * Read/write file and return byte count read/written
 * 
 * @return On success: How many bytes have been read/written. 
 * On error: -1
 * */
PUBLIC int do_rdwt(){
    int fd = fs_msg.FD;
    void* buf = fs_msg.BUF;
    int len = fs_msg.CNT;

    int src = fs_msg.source;
    kprintf("pcaller fd: %d", fd);
    assert((pcaller->filp[fd] >= &f_desc_table[0]) 
        && (pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));
    
    if(!(pcaller->filp[fd]->fd_mode & O_RDWR))
        return -1;

    int pos = pcaller->filp[fd]->fd_pos;
    struct inode* pin = pcaller->filp[fd]->fd_inode;

    assert(pin >= &inode_table[0] && pin < &inode_table[NR_INODE]);

    int imode = pin->i_mode & I_TYPE_MASK;
    // TODO: wrap into do_rdwt_special
    if(imode == I_CHAR_SPECIAL){ // special file, such as tty
        kprintf("write to I CHAR SPECIAL");
        int t = fs_msg.type == READ? DEV_READ : DEV_WRITE;
        fs_msg.type = t;

        int dev = pin->i_start_sect; // this field means different for special file/device
        assert(MAJOR(dev) == 4);

        fs_msg.DEVICE = MINOR(dev);
        fs_msg.BUF = buf;
        fs_msg.CNT = len;
        fs_msg.PROC_NR = src;
        assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

        send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &fs_msg);
        assert(fs_msg.CNT == len);

        return fs_msg.CNT;

    }else{ // regular file // TODO: wrap into do_rdwt_regular()
        assert(pin->i_mode == I_REGULAR || pin->i_mode == I_DIRECTORY);
        assert(fs_msg.type == READ || fs_msg.type == WRITE);

        int pos_end;
        if(fs_msg.type == READ)
            pos_end = min(pos + len, pin->i_size);
        else //WRITE
            pos_end = min(pos + len, pin->i_nr_sects * SECTOR_SIZE);

        int off = pos % SECTOR_SIZE;
        int rw_sect_min = pin->i_start_sect + (pos >> SECTOR_SIZE_SHIFT);
        int rw_sect_max = pin->i_start_sect + (pos_end >> SECTOR_SIZE_SHIFT);

        int chunk = min(rw_sect_max - rw_sect_min + 1, FSBUF_SIZE >> SECTOR_SIZE_SHIFT);

        int bytes_rw = 0;
        int bytes_left = len;
        int i;
        for(i = rw_sect_min; i<= rw_sect_max; i += chunk){
            // read/write this amount of bytes each loop
            int bytes = min(bytes_left, chunk*SECTOR_SIZE - off); // TODO:??

            rw_sector(DEV_READ, pin->i_dev, i*SECTOR_SIZE, chunk*SECTOR_SIZE, TASK_FS, fsbuf);

            if(fs_msg.type == READ){
                phys_copy((void*)va2la(src, buf + bytes_rw),
                    (void*)va2la(TASK_FS, fsbuf + off), bytes);
            }else { // WRITE
                phys_copy((void*)va2la(TASK_FS, fsbuf + off),
                    (void*)va2la(src, buf + bytes_rw), bytes);
                kprintf(">>> 8.2 in do_rwrt(), before rw_sector()\n");
                rw_sector(DEV_WRITE, pin->i_dev, i*SECTOR_SIZE, chunk*SECTOR_SIZE, TASK_FS, fsbuf);                
                kprintf(">>> 8.2 in do_rwrt(), after rw_sector()\n");
            }
            off = 0;
            bytes_rw += bytes;
            pcaller->filp[fd]->fd_pos += bytes;
            bytes_left -= bytes;
        }

        if(pcaller->filp[fd]->fd_pos > pin->i_size){
            // update inode size
            pin->i_size = pcaller->filp[fd]->fd_pos;
            sync_inode(pin);
        }

        return bytes_rw;
    }
}

/**
 * @return on success zero is returned, on error, -1 is returned.
 * */
PUBLIC int do_unlink(){
    char pathname[MAX_PATH];
    int name_len = fs_msg.NAME_LEN;
    int src = fs_msg.source;

    assert(name_len < MAX_PATH);
    phys_copy((void*)va2la(TASK_FS, pathname),
        (void*)va2la(src, fs_msg.PATHNAME),
        name_len);
    pathname[name_len] = 0;

    if(strcmp(pathname, "/") == 0){
        kprintf("fs::do_unlink cannot unlink the root\n");
        return -1;
    }

    int inode_nr = search_file(pathname);
    if(inode_nr == INVALID_INODE){
        kprintf("fs::unlink search file:%s returns invalid inode\n", pathname);
        return -1;
    }

    char filename[MAX_PATH];
    struct inode* dir_inode;
    if(strip_path(filename, pathname, &dir_inode) != 0){
        kprintf("fs::unlink strip path:%s failed\n", pathname);
        return -1;
    }

    struct inode* pin = get_inode(dir_inode->i_dev, inode_nr);

    if(pin->i_mode != I_REGULAR){ // can only remove regular files
        kprintf("cannot move non regular file.");
        return -1;
    }

    if(pin->i_cnt > 1){ 
        kprintf("cannot remove file:%s, pin->i_cnt is %d\n", pathname, pin->i_cnt);
        return -1;
    }

    struct super_block* sb = get_super_block(pin->i_dev);

    // ============= free the bit in i-map
    int byte_idx = inode_nr / 8;
    int bit_idx = inode_nr % 8;
    assert(byte_idx < SECTOR_SIZE); // we have only one i-map sector
    // read sector 2, skip boot sector and superblock sector
    RD_SECT(pin->i_dev, 2);
    assert(fsbuf[byte_idx % SECTOR_SIZE] & (1 << bit_idx));
    fsbuf[byte_idx % SECTOR_SIZE] &= ~(1 << bit_idx);
    WR_SECT(pin->i_dev, 2);
    
    // ============= free the bit in s-map
    /*
	 *           bit_idx: bit idx in the entire i-map
	 *     ... ____|____
	 *                  \        .-- byte_cnt: how many bytes between
	 *                   \      |              the first and last byte
	 *        +-+-+-+-+-+-+-+-+ V +-+-+-+-+-+-+-+-+
	 *    ... | | | | | |*|*|*|...|*|*|*|*| | | | |
	 *        +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
	 *         0 1 2 3 4 5 6 7     0 1 2 3 4 5 6 7
	 *  ...__/
	 *      byte_idx: byte idx in the entire i-map
	 */
    bit_idx = pin->i_start_sect - sb->n_1st_sect + 1;
    byte_idx = bit_idx / 8;
    int bits_left = pin->i_nr_sects;
    int byte_cnt = (bits_left - (8 - (bit_idx % 8))) / 8;

    // current sector number
    int s = 1 + 1 + sb->nr_imap_sects + byte_idx / SECTOR_SIZE;
    RD_SECT(pin->i_dev, s);

    int i;
	/* clear the first byte */
	for (i = bit_idx % 8; (i < 8) && bits_left; i++,bits_left--) {
		assert((fsbuf[byte_idx % SECTOR_SIZE] >> i & 1) == 1);
		fsbuf[byte_idx % SECTOR_SIZE] &= ~(1 << i);
	}

	/* clear bytes from the second byte to the second to last */
	int k;
	i = (byte_idx % SECTOR_SIZE) + 1;	/* the second byte */
	for (k = 0; k < byte_cnt; k++,i++,bits_left-=8) {
		if (i == SECTOR_SIZE) {
			i = 0;
			WR_SECT(pin->i_dev, s);
			RD_SECT(pin->i_dev, ++s);
		}
		assert(fsbuf[i] == 0xFF);
		fsbuf[i] = 0;
	}

	/* clear the last byte */
	if (i == SECTOR_SIZE) {
		i = 0;
		WR_SECT(pin->i_dev, s);
		RD_SECT(pin->i_dev, ++s);
	}
	unsigned char mask = ~((unsigned char)(~0) << bits_left);
	assert((fsbuf[i] & mask) == mask);
	fsbuf[i] &= (~0) << bits_left;
	WR_SECT(pin->i_dev, s);

    // ============= clear inode itself
    pin->i_mode = 0;
    pin->i_size = 0;
    pin->i_start_sect = 0;
    pin->i_nr_sects = 0;
    sync_inode(pin);
    put_inode(pin); // release slot in inode_table

    // ============= set inode_nr to 0 in directory entry
    int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	int nr_dir_entries =
		dir_inode->i_size / DIR_ENTRY_SIZE; /* including unused slots
						     * (the file has been
						     * deleted but the slot
						     * is still there)
						     */
	int m = 0;
	struct dir_entry * pde = 0;
	int flg = 0;
	int dir_size = 0;

	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

		pde = (struct dir_entry *)fsbuf;
		int j;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (++m > nr_dir_entries)
				break;

			if (pde->inode_nr == inode_nr) {
				/* pde->inode_nr = 0; */
				memset(pde, 0, DIR_ENTRY_SIZE);
				WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);
				flg = 1;
				break;
			}

			if (pde->inode_nr != INVALID_INODE)
				dir_size += DIR_ENTRY_SIZE;
		}

		if (m > nr_dir_entries || /* all entries have been iterated OR */
		    flg) /* file is found */
			break;
	}
	assert(flg);
	if (m == nr_dir_entries) { /* the file is the last one in the dir */
		dir_inode->i_size = dir_size;
		sync_inode(dir_inode);
	}

    return 0;
}