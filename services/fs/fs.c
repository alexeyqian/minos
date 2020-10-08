#include "_fs.h"
#include <minos/hd.h>
#include <minos/fs.h>

// TODO: use dynamic allocation for these buffers
#define FSBUF_BASE 0x100000
#define FSBUF_SIZE 0X100000
PRIVATE char*  fsbuf = (char*)FSBUF_BASE;

struct procfd{
    struct file_desc* filp[NR_FILES];
};

PRIVATE struct file_desc     fdesc_table[NR_FILE_DESC];
PRIVATE struct procfd        procfd_table[PROCTABLE_SIZE];
PRIVATE struct inode         inode_table[NR_INODE];
PRIVATE struct super_block   superblock_table[NR_SUPER_BLOCK]; 

// remember to modify include/const.h if the order is changed
PRIVATE struct dev_drv_map dd_map[] = {
    {INVALID_DRIVER}, // unused
    {INVALID_DRIVER}, // reserved for floppy driver
    {INVALID_DRIVER}, // reserved for cdrom
    {TASK_HD},        // hard disk: driver is task_hd
    {TASK_TTY},       // tty
    {INVALID_DRIVER}  // reserved for scsi disk driver
};

PUBLIC int get_dev_driver(int dev){
    int retval = dd_map[MAJOR(dev)].driver_nr;
    assertx(retval != INVALID_DRIVER);
    return retval;
}

PRIVATE void reset_filedesc_table(){
    for(int i = 0; i < NR_FILE_DESC; i++)
        memset(&fdesc_table[i], 0, sizeof(struct file_desc));
}

PRIVATE void reset_procfd_table(){
    for(int i = 0; i < PROCTABLE_SIZE; i++){
        for(int j = 0; j < NR_FILES; j++)
            procfd_table[i].filp[j] = 0;
    }
}

PRIVATE void reset_inode_table(){
    for(int i = 0; i < NR_INODE; i++)
        memset(&inode_table[i], 0, sizeof(struct inode));
}

PRIVATE void reset_superblock_table(){
    struct super_block* sb = superblock_table;
    for(; sb < &superblock_table[NR_SUPER_BLOCK]; sb++)
        sb->sb_dev = NO_DEV;
}

PUBLIC int is_valid_inode(struct inode* pin){
    if(pin >= &inode_table[0] && pin < &inode_table[NR_INODE])
        return 1;
    else 
        return 0;
}

PUBLIC struct super_block* get_super_block(int dev){
    struct super_block* sb = superblock_table;
    for(; sb < &superblock_table[NR_SUPER_BLOCK]; sb++){
        if(sb->sb_dev == dev) {
            assertx(sb->magic == MAGIC_V1);
            return sb;
        }
    }

    panicx("super block of device %d not found.\n", dev);
    return 0;
}

PUBLIC void load_super_block(int dev){
    int i;
    KMESSAGE driver_msg;

    driver_msg.type = DEV_READ;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = SECTOR_SIZE * 1;
    driver_msg.BUF = fsbuf;
    driver_msg.CNT = SECTOR_SIZE;
    driver_msg.PROC_NR = TASK_FS;
    send_recv(BOTH, get_dev_driver(dev), &driver_msg);

    // find a free slot in super block table
    for(i = 0; i < NR_SUPER_BLOCK; i++){
        if(superblock_table[i].sb_dev == NO_DEV) break;
    }

    if(i == NR_SUPER_BLOCK) 
        panicx("super block slots used up.\n");
    
    assertx(i == 0); // currently we only use the 1st slot

    struct super_block* psb = (struct super_block*)fsbuf;
    superblock_table[i] = *psb; // TODO: copy data over ??
    superblock_table[i].sb_dev = dev;    
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
 * @return zero if success, otherwise kpanic
 */
PUBLIC int rw_sector(int io_type, int dev, uint64_t pos, int bytes, int proc_nr, void* buf){
    KMESSAGE driver_msg;
    driver_msg.type = io_type;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = pos;
    driver_msg.BUF = buf;
    driver_msg.CNT = bytes;
    driver_msg.PROC_NR = proc_nr;
    send_recv(BOTH, get_dev_driver(dev), &driver_msg);
    return 0;
}


/** 
 * get the inode ptr of given inode number.
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
            // TODO: break here?
        }
    }

    if(!q) panicx("the inode table is full.");

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
 *      - *ppinode: rootinode
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
    *ppinode = get_inode(ROOT_DEV, ROOT_INODE);
    return 0;
}

/** Decrease the reference number of a slot in inode table
 * when the number reaches zero, it means the inode is not used 
 * any more and can be overwritten by a new inode.
 * */
PRIVATE void put_inode(struct inode* pinode){
    assertx(pinode->i_cnt > 0);
    pinode->i_cnt--;
}

/** write the inode back to the disk
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
            if((uint8_t)fsbuf[j] == 0xFF) continue;

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

    panicx("inode map is probably full.\n");
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
				if ((uint8_t)fsbuf[j] == 0xFF) continue;
				for (; ((fsbuf[j] >> k) & 1) != 0; k++) {}
				free_sect_nr = (i * SECTOR_SIZE + j) * 8 +
					k - 1 + sb->n_1st_sect;
			}

            for(; k < 8; k++){ // repeat until enough bits are set
                assertx(((fsbuf[j] >> k) & 1) == 0);
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


    assertx(nr_sects_to_alloc == 0);
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

// open a file and return the file descriptor
// return -1 if failed
PUBLIC int do_open(KMESSAGE* pmsg, struct procfd* caller){
    int fd = -1;
    char pathname[MAX_PATH];

    int flags = pmsg->FLAGS;
    int name_len = pmsg->NAME_LEN;
    int src = pmsg->source; // caller proc number
    assertx(name_len < MAX_PATH);
    phys_copy((void*)va2la(TASK_FS, pathname),
        (void*)va2la(src, pmsg->PATHNAME),
        name_len);
    pathname[name_len] = 0;

    // find a free slot in proc's filp[]
    int i;
    for(i = 0; i < NR_FILES; i++){
        if(caller->filp[i] == 0){
            fd = i;
            break;
        }
    }

    if((fd < 0) || (fd >= NR_FILES)){
        panicx("filp[] is full (pid: %d)", pmsg->source);
    }
        
    // find a free slot in file descriptor table
    for(i = 0; i < NR_FILE_DESC; i++){
        if(fdesc_table[i].fd_inode == 0)
            break;
    }
    
    if(i >= NR_FILE_DESC)
        panicx("file descriptor table is full, pid: %d", pmsg->source);

    int inode_nr = search_file(pathname);
    struct inode* pin = 0;
    if(flags & O_CREAT){
        if(inode_nr){
            printx("file exists.\n");
            return -1;
        }else{
            pin = create_file(pathname, flags);
        }
    }else{
        assertx(flags & O_RDWR);
        
        char filename[MAX_PATH];
        struct inode* dir_inode;
        if(strip_path(filename, pathname, &dir_inode) != 0)
            return -1;
        pin = get_inode(dir_inode->i_dev, inode_nr);
    }

    if(pin){        
        // connects proc with file_descriptor
        caller->filp[fd] = &fdesc_table[i]; // TODO: rememver the mapping between caller and fdesc_table
        // fd_entry_table:fd_entry{p_idx, pf_idx, fdt_idx}
        // TODO: syscall: set_proc_fd(p_idx, pf_idx, fdt_idx)
        // connects file_descriptor with inode
        fdesc_table[i].fd_inode = pin;
        fdesc_table[i].fd_mode = flags;
        fdesc_table[i].fd_cnt = 1;
        fdesc_table[i].fd_pos = 0;

        int imode = pin->i_mode & I_TYPE_MASK;
        if(imode == I_CHAR_SPECIAL){
            KMESSAGE driver_msg;
            driver_msg.type = DEV_OPEN;
            int dev = pin->i_start_sect;
            driver_msg.DEVICE = MINOR(dev);
            assertx(MAJOR(dev) == 4);
            send_recv(BOTH, get_dev_driver(dev), &driver_msg);
        }else if(imode == I_DIRECTORY){
            assertx(pin->i_num == ROOT_INODE);
        }else{
            assertx(pin->i_mode == I_REGULAR);
        }

    }else{
        return -1;
    }

    return fd;
}

PUBLIC int do_close(int fd, struct procfd* caller){    
    // TODO: use fdesc_table instead of caller->filp[fd] to avoid use pcaller
    put_inode(caller->filp[fd]->fd_inode); // inode->cnt--
    if(--caller->filp[fd]->fd_cnt == 0)
        caller->filp[fd]->fd_inode = 0;    // disconnect file descriptor from inode 
    caller->filp[fd] = 0;                  // disconnect fd from file descriptors

    return 0;
}

// the new offset in bytes from the beginning of the file if succesfully
// otherwise a negative number.
PUBLIC int do_lseek(struct kmessage* pmsg, struct procfd* caller){
    int fd = pmsg->FD;
    int off = pmsg->OFFSET;
    int whence = pmsg->WHENCE;

    int pos = caller->filp[fd]->fd_pos;
    int f_size = caller->filp[fd]->fd_inode->i_size;

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

    caller->filp[fd]->fd_pos = pos;
    return pos;
}

/**
 * Read/write file and return byte count read/written
 * 
 * @return On success: How many bytes have been read/written. 
 * On error: -1
 * */
PUBLIC int do_rdwt(struct kmessage* pmsg, struct procfd* caller){
    int fd = pmsg->FD;
    void* buf = pmsg->BUF;
    int len = pmsg->CNT;

    int src = pmsg->source;
    assertx((caller->filp[fd] >= &fdesc_table[0]) 
        && (caller->filp[fd] < &fdesc_table[NR_FILE_DESC]));
    
    if(!(caller->filp[fd]->fd_mode & O_RDWR))
        return -1;

    int pos = caller->filp[fd]->fd_pos;
    struct inode* pin = caller->filp[fd]->fd_inode;
    
    assertx(is_valid_inode(pin));
    int imode = pin->i_mode & I_TYPE_MASK;
    if(imode == I_CHAR_SPECIAL){ // special file, such as tty
        // TODO: create a new msg
        int t = pmsg->type == READ? DEV_READ : DEV_WRITE;
        pmsg->type = t;
        //printx("new msg type: %d", msg->type);
        int dev = pin->i_start_sect; // this field means different for special file/device
        assertx(MAJOR(dev) == 4);

        pmsg->DEVICE = MINOR(dev);
        pmsg->BUF = buf;
        pmsg->CNT = len;
        pmsg->PROC_NR = src;
        send_recv(BOTH, get_dev_driver(dev), pmsg);
        assertx(pmsg->CNT == len);

        return pmsg->CNT;

    }else{ // regular file 
        assertx(pin->i_mode == I_REGULAR || pin->i_mode == I_DIRECTORY);
        assertx(pmsg->type == READ || pmsg->type == WRITE);

        int pos_end;
        if(pmsg->type == READ)
            pos_end = min((uint32_t)(pos + len), pin->i_size);
        else //WRITE
            pos_end = min((uint32_t)(pos + len), pin->i_nr_sects * SECTOR_SIZE);

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

            if(pmsg->type == READ){
                phys_copy((void*)va2la(src, buf + bytes_rw),
                    (void*)va2la(TASK_FS, fsbuf + off), bytes);
            }else { // WRITE
                phys_copy((void*)va2la(TASK_FS, fsbuf + off),
                    (void*)va2la(src, buf + bytes_rw), bytes);
                rw_sector(DEV_WRITE, pin->i_dev, i*SECTOR_SIZE, chunk*SECTOR_SIZE, TASK_FS, fsbuf);                
            }
            off = 0;
            bytes_rw += bytes;
            caller->filp[fd]->fd_pos += bytes;
            bytes_left -= bytes;
        }

        if((uint32_t)caller->filp[fd]->fd_pos > pin->i_size){ 
            // update inode size
            pin->i_size = caller->filp[fd]->fd_pos;
            sync_inode(pin);
        }
        return bytes_rw;
    }
}

/**
 * @return on success zero is returned, on error, -1 is returned.
 * */
PUBLIC int do_unlink(struct kmessage* msg){
    char pathname[MAX_PATH];
    int name_len = msg->NAME_LEN;
    int src = msg->source;

    assertx(name_len < MAX_PATH);
    phys_copy((void*)va2la(TASK_FS, pathname),
        (void*)va2la(src, msg->PATHNAME),
        name_len);
    pathname[name_len] = 0;

    if(strcmp(pathname, "/") == 0){
        printx("fs::do_unlink cannot unlink the root\n");
        return -1;
    }

    int inode_nr = search_file(pathname);
    if(inode_nr == INVALID_INODE){
        printx("fs::unlink search file:%s returns invalid inode\n", pathname);
        return -1;
    }

    char filename[MAX_PATH];
    struct inode* dir_inode;
    if(strip_path(filename, pathname, &dir_inode) != 0){
        printx("fs::unlink strip path:%s failed\n", pathname);
        return -1;
    }

    struct inode* pin = get_inode(dir_inode->i_dev, inode_nr);

    if(pin->i_mode != I_REGULAR){ // can only remove regular files
        printx("cannot move non regular file.");
        return -1;
    }

    if(pin->i_cnt > 1){ 
        printx("cannot remove file:%s, pin->i_cnt is %d\n", pathname, pin->i_cnt);
        return -1;
    }

    struct super_block* sb = get_super_block(pin->i_dev);

    // ============= free the bit in i-map
    int byte_idx = inode_nr / 8;
    int bit_idx = inode_nr % 8;
    assertx(byte_idx < SECTOR_SIZE); // we have only one i-map sector
    // read sector 2, skip boot sector and superblock sector
    RD_SECT(pin->i_dev, 2);
    assertx(fsbuf[byte_idx % SECTOR_SIZE] & (1 << bit_idx));
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
		assertx((fsbuf[byte_idx % SECTOR_SIZE] >> i & 1) == 1);
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
		assertx(fsbuf[i] == 0xFF);
		fsbuf[i] = 0;
	}

	/* clear the last byte */
	if (i == SECTOR_SIZE) {
		i = 0;
		WR_SECT(pin->i_dev, s);
		RD_SECT(pin->i_dev, ++s);
	}
	unsigned char mask = ~((uint8_t)(~0) << bits_left);
	assertx((fsbuf[i] & mask) == mask);
	fsbuf[i] &= (uint8_t)(~0) << bits_left;
	WR_SECT(pin->i_dev, s);

    // ============= clear inode itself
    pin->i_mode = 0;
    pin->i_size = 0;
    pin->i_start_sect = 0;
    pin->i_nr_sects = 0;
    sync_inode(pin);
    put_inode(pin); // release slot in inode table

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
		for (j = 0; j < SECTOR_SIZE / (int)DIR_ENTRY_SIZE; j++,pde++) {
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
	assertx(flg);
	if (m == nr_dir_entries) { /* the file is the last one in the dir */
		dir_inode->i_size = dir_size;
		sync_inode(dir_inode);
	}

    return 0;
}

PUBLIC int do_stat(KMESSAGE* pmsg){
    char pathname[MAX_PATH];
    char filename[MAX_PATH];

    int name_len = pmsg->NAME_LEN;
    int src = pmsg->source;
    assertx(name_len < MAX_PATH);
    phys_copy((void*)va2la(TASK_FS, pathname),
              (void*)va2la(src, pmsg->PATHNAME),
              name_len);
    pathname[name_len] = 0;

    int inode_nr = search_file(pathname);
    if(inode_nr == INVALID_INODE){
        printx("fs::do_stat search file returns invalid inode %s\n", pathname);
        return -1;
    }

    struct inode* pin = 0;
    struct inode* dir_inode;
    if(strip_path(filename, pathname, &dir_inode) != 0){
        assertx(0);
    }

    pin = get_inode(dir_inode->i_dev, inode_nr);

    struct stat s;
    s.st_dev = pin->i_dev;
    s.st_ino = pin->i_num;
    s.st_mode = pin->i_mode;
    s.st_rdev = is_special(pin->i_mode) ? pin->i_start_sect : NO_DEV;
    s.st_size = pin->i_size;

    put_inode(pin);

    phys_copy((void*)va2la(src, pmsg->BUF),
              (void*)va2la(TASK_FS, &s),
              sizeof(struct stat));

    return 0;
}

// CALLED FROM SYSTEM
PRIVATE int fs_fork_process(int child_pid){
    int i;
    struct procfd* child = &procfd_table[child_pid];
    for(i = 0; i < NR_FILES; i++){
        if(child->filp[i]){
            child->filp[i]->fd_cnt++;
            child->filp[i]->fd_inode->i_cnt++;
        }
    }

    return 0;
}

// CALLED FROM SYSTEM
PRIVATE int fs_exit_process(struct procfd* p){    
    for(int i = 0; i < NR_FILES; i++){
        if(p->filp[i]){
            // release inode
            p->filp[i]->fd_inode->i_cnt--;
            // release file descriptor
            if(--p->filp[i]->fd_cnt == 0)
                p->filp[i]->fd_inode = 0;
            p->filp[i] = 0;
        }
    }

    return 0;
}

PRIVATE void set_super_block(struct super_block* sb){
    // get the geometry of ROOTDEV
    KMESSAGE driver_msg;    
    struct part_info geo;
    driver_msg.type = DEV_IOCTL;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    driver_msg.REQUEST = DIOCTL_GET_GEO;
    driver_msg.BUF = &geo;
    driver_msg.PROC_NR = TASK_FS;
    send_recv(BOTH, get_dev_driver(ROOT_DEV), &driver_msg);
    printx("dev size: 0x%x sectors\n", geo.size);

    uint32_t bits_per_sect = SECTOR_SIZE * 8;
    sb->magic = MAGIC_V1;
    sb->nr_inodes = bits_per_sect; // maximum nodes
    sb->nr_inode_sects = sb->nr_inodes * INODE_SIZE / SECTOR_SIZE;
    sb->nr_sects = geo.size; // partition size in sector
    sb->nr_imap_sects = 1;   // total inode map size in sector, max inodes = 4096 = 512*8
    sb->nr_smap_sects = sb->nr_sects / bits_per_sect + 1;
    sb->n_1st_sect = 1 + 1 // boot sector $ super block
        + sb->nr_imap_sects + sb->nr_smap_sects + sb->nr_inode_sects;
    sb->root_inode = ROOT_INODE; // = 1, inode 0 is reserved
    sb->inode_size = INODE_SIZE;
    struct inode x;
    sb->inode_isize_off = (int)&x.i_size - (int)&x;
    sb->inode_start_off = (int)&x.i_start_sect - (int)&x;
    sb->dir_ent_size = DIR_ENTRY_SIZE;
    struct dir_entry de;
    sb->dir_ent_inode_off = (int)&de.inode_nr - (int)&de;
    sb->dir_ent_fname_off = (int)&de.name - (int)&de;

    memset(fsbuf, 0x90, SECTOR_SIZE);
    memcpy(fsbuf, sb, SUPER_BLOCK_SIZE); // NOT &sb

    // write the super block to sector 1
    WR_SECT(ROOT_DEV, 1);
    
    printx("dev base:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
	       "        inodes:0x%x00, 1st_sector:0x%x00\n", 
	       geo.base * 2,
	       (geo.base + 1) * 2,
	       (geo.base + 1 + 1) * 2,
	       (geo.base + 1 + 1 + sb->nr_imap_sects) * 2,
	       (geo.base + 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects) * 2,
	       (geo.base + sb->n_1st_sect) * 2);
      
}

PRIVATE void set_inode_map(){
    memset(fsbuf, 0, SECTOR_SIZE);
    for(int i = 0; i < (NR_CONSOLES + 3); i++)
        fsbuf[0] |= 1 << i;

    // bit 0: reserved
    // bit 1: the first inode which indicates '/'
    // bit 2: /dev_tty0
    // bit 3: /dev_tty1
    // bit 4: /dev_tty2
    // bit 5: /inst.tar
    assertx(fsbuf[0] == 0x3F); // 0011 1111

    // write inode map to sector 2
    WR_SECT(ROOT_DEV, 2);
}

PRIVATE void set_sector_map(struct super_block* sb){
    int i, j;
    memset(fsbuf, 0, SECTOR_SIZE);
    // bit 0  is reserved (sector 0),
    // NR_DEFAULT_FILE_SECTS is reserved to '/'
    int nr_sects = NR_DEFAULT_FILE_SECTS + 1; 

    for(i = 0; i < nr_sects / 8; i++)
        fsbuf[i] = 0xFF;
    
    for(j = 0; j < nr_sects % 8; j++)
        fsbuf[i] |= (1 << j); // is i not j

    // write sector map to sector n:2+nr_imap_sects
    WR_SECT(ROOT_DEV, 2 + sb->nr_imap_sects);

    // zero the rest sector map
    memset(fsbuf, 0, SECTOR_SIZE);
    for(i = 1; i < (int)sb->nr_smap_sects; i++)
        WR_SECT(ROOT_DEV, 2 + sb->nr_imap_sects + i);
    
    // set sector map for inst.tar
	// make sure it'll not be overwritten by the disk log 
	//assert(INSTALL_START_SECT + INSTALL_NR_SECTS < sb->nr_sects - NR_SECTS_FOR_LOG);
	int bit_offset = INSTALL_START_SECT -
		sb->n_1st_sect + 1; /* sect M <-> bit (M - sb->n_1stsect + 1) */
	int bit_off_in_sect = bit_offset % (SECTOR_SIZE * 8);
	int bit_left = INSTALL_NR_SECTS;
	int cur_sect = bit_offset / (SECTOR_SIZE * 8);
	RD_SECT(ROOT_DEV, 2 + sb->nr_imap_sects + cur_sect);
	while (bit_left) {
		int byte_off = bit_off_in_sect / 8;
		/* this line is ineffecient in a loop, but I don't care */
		fsbuf[byte_off] |= 1 << (bit_off_in_sect % 8);
		bit_left--;
		bit_off_in_sect++;
		if (bit_off_in_sect == (SECTOR_SIZE * 8)) {
			WR_SECT(ROOT_DEV, 2 + sb->nr_imap_sects + cur_sect);
			cur_sect++;
			RD_SECT(ROOT_DEV, 2 + sb->nr_imap_sects + cur_sect);
			bit_off_in_sect = 0;
		}
	}
	WR_SECT(ROOT_DEV, 2 + sb->nr_imap_sects + cur_sect);
}

PRIVATE void set_inodes(struct super_block* sb){
    // inode of '/'
    memset(fsbuf, 0, SECTOR_SIZE);
    struct inode * pi = (struct inode*)fsbuf;
    pi->i_mode = I_DIRECTORY;
    pi->i_size = DIR_ENTRY_SIZE * 5; // 5 files . dev_tty0 - 2, inst.tar

    pi->i_start_sect = sb->n_1st_sect; // first data sect
    pi->i_nr_sects = NR_DEFAULT_FILE_SECTS;

    // inode of /dev_tty0~2
    for(int i = 0; i < NR_CONSOLES; i++){
        pi = (struct inode*)(fsbuf + (INODE_SIZE * (i + 1)));
        pi->i_mode = I_CHAR_SPECIAL;
        pi->i_size = 0;
        pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
        pi->i_nr_sects = 0;
    }    

    //inode of `/inst.tar'
	pi = (struct inode*)(fsbuf + (INODE_SIZE * (NR_CONSOLES + 1)));
	pi->i_mode = I_REGULAR;
	pi->i_size = INSTALL_NR_SECTS * SECTOR_SIZE;
	pi->i_start_sect = INSTALL_START_SECT;
	pi->i_nr_sects = INSTALL_NR_SECTS;

    // write inodes array sector
    WR_SECT(ROOT_DEV, 2 + sb->nr_imap_sects + sb->nr_smap_sects);

}

PRIVATE void set_root_dir(struct super_block* sb){
    memset(fsbuf, 0, SECTOR_SIZE);
    struct dir_entry* pde = (struct dir_entry*)fsbuf;
    pde->inode_nr = 1;
    strcpy(pde->name, ".");

    // dir entry of /dev_tty0~2
    for(int i = 0; i < NR_CONSOLES; i++){
        pde++;
        pde->inode_nr = i + 2;// dev_tty0's inode is 2
        sprintf(pde->name, "dev_tty%d", i);
    }

    (++pde)->inode_nr = NR_CONSOLES + 2;
	strcpy(pde->name, "inst.tar");
    
    // write '/' directory file into first data sector
    WR_SECT(ROOT_DEV, sb->n_1st_sect);    
}


/** <ring 1> 
 * write a super block to sector 1
 * create 3 special files: dev_tty0, dev_tty1, dev_tty2
 * create the inode map
 * create the sector map
 * create the inodes of the files
 * create /, the root directory
 */
// TODO: add param buf to replace global var fsbuf
PRIVATE void mkfs(){
    struct super_block sb;
    set_super_block(&sb);
    set_inode_map();
    set_sector_map(&sb);
    set_inodes(&sb);    
    set_root_dir(&sb);    
}

PRIVATE void open_hd(){
    KMESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    send_recv(BOTH, get_dev_driver(ROOT_DEV), &driver_msg);
}

PRIVATE void init_fs(){
    reset_filedesc_table();
    reset_procfd_table();
    reset_inode_table();    
    reset_superblock_table();
    open_hd();
    mkfs();
    load_super_block(ROOT_DEV);
}

PUBLIC void svc_fs(){
    printx(">>> 4. service fs is running.\n");
    init_fs();
    KMESSAGE fs_msg; 
    struct procfd* pcaller;
    while(TRUE){
        send_recv(RECEIVE, ANY, &fs_msg);    

        int msgtype = fs_msg.type;
        int src = fs_msg.source;
        pcaller = &procfd_table[src];  // TODO: pcaller = get_proc(src);
        switch(msgtype){
            case OPEN:
                //printx(" do_open() begin: flags: %d, recv from: %d\n", pcaller->p_flags, pcaller->p_recvfrom);
                fs_msg.FD = do_open(&fs_msg, pcaller);
                //printx(" do_open() end: flags: %d, recv from: %d\n", pcaller->p_flags, pcaller->p_recvfrom);
                break;
            case CLOSE:
                fs_msg.RETVAL = do_close(fs_msg.FD, pcaller); // should do syscall here?
                break;
            case READ:
            case WRITE:
                fs_msg.CNT = do_rdwt(&fs_msg, pcaller);
                break;
            case UNLINK:
                fs_msg.RETVAL = do_unlink(&fs_msg);
                break;
            case LSEEK: 
                fs_msg.OFFSET = do_lseek(&fs_msg, pcaller);
                break; 
            case RESUME_PROC:
                src = fs_msg.PROC_NR;
             	break;
            case FORK:
             	fs_msg.RETVAL = fs_fork_process(fs_msg.PID); 
             	break; 
            case EXIT: 
             	fs_msg.RETVAL = fs_exit_process(pcaller); 
             	break; 
            case STAT: 
             	fs_msg.RETVAL = do_stat(&fs_msg); 
             	break; 
            default:
                //dump_msg("fs: unknow message: ", &fs_msg); // TODO: implement dump_msg in user space
                assertx(0);
                break;
        }   

        // reply
        // when fs receive SUSPEND_PROC message, it just ignore it,
        // so the requesting process P (= fs_msg.PROC_NR) will wait, until fs receives RESUME_PROC message
        // it then notify process P to let it continue.
        if(fs_msg.type != SUSPEND_PROC){
            fs_msg.type = SYSCALL_RET;  
            //struct proc* temp_proc = &proc_table[src];       
            //printx(">>> fs_tty::send back begin, src: %d, flags: %d, recvfrom: %d, sendto:%d\n", 
            //    src, temp_proc->p_flags, temp_proc->p_recvfrom, temp_proc->p_sendto);               
            send_recv(SEND, src, &fs_msg);
            //printx(">>> fs_tty::send back end, src: %d\n", src);
        }        
    }
}
