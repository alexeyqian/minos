#include "fs.h"
#include "./fs_open.h"
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

PRIVATE void read_super_block(int dev){
    int i;
    MESSAGE driver_msg;

    driver_msg.type = DEV_READ;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = SECTOR_SIZE * 1;
    driver_msg.BUF = fsbuf;
    driver_msg.CNT = SECTOR_SIZE;
    driver_msg.PROC_NR = TASK_FS;

    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);

    // find a free slot in super_block[]
    for(i = 0; i < NR_SUPER_BLOCK; i++){
        if(super_block[i].sb_dev == NO_DEV) break;
    }

    if(i == NR_SUPER_BLOCK) 
        panic("super_block slots used up.\n");
    
    assert(i == 0); // currently we only use the 1st slot

    struct super_block* psb = (struct super_block*)fsbuf;
    super_block[i] = *psb; // TODO: copy data over ??
    super_block[i].sb_dev = dev;
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
// TODO: remove global vars
PRIVATE void mkfs(){
    MESSAGE driver_msg;
    uint32_t i, j;
    uint32_t bits_per_sect = SECTOR_SIZE * 8;
    
    // get the geometry of ROOTDEV
    struct part_info geo;
    driver_msg.type = DEV_IOCTL;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    driver_msg.REQUEST = DIOCTL_GET_GEO;
    driver_msg.BUF = &geo;
    driver_msg.PROC_NR = TASK_FS;

    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    printl("dev size: 0x%x sectors\n", geo.size);

    //========= super block ============
    struct super_block sb;
    sb.magic = MAGIC_V1;
    sb.nr_inodes = bits_per_sect; // maximum nodes
    sb.nr_inode_sects = sb.nr_inodes * INODE_SIZE / SECTOR_SIZE;
    sb.nr_sects = geo.size; // partition size in sector
    sb.nr_imap_sects = 1;   // total inode map size in sector, max inodes = 4096 = 512*8
    sb.nr_smap_sects = sb.nr_sects / bits_per_sect + 1;
    sb.n_1st_sect = 1 + 1 // boot sector $ super block
        + sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects;
    sb.root_inode = ROOT_INODE; // = 1, inode 0 is reserved
    sb.inode_size = INODE_SIZE;
    struct inode x;
    sb.inode_isize_off = (int)&x.i_size - (int)&x;
    sb.inode_start_off = (int)&x.i_start_sect - (int)&x;
    sb.dir_ent_size = DIR_ENTRY_SIZE;
    struct dir_entry de;
    sb.dir_ent_inode_off = (int)&de.inode_nr - (int)&de;
    sb.dir_ent_fname_off = (int)&de.name - (int)&de;

    memset(fsbuf, 0x90, SECTOR_SIZE);
    memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

    // write the super block to sector 1
    WR_SECT(ROOT_DEV, 1);

    printl("devbase:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
	       "        inodes:0x%x00, 1st_sector:0x%x00\n", 
	       geo.base * 2,
	       (geo.base + 1) * 2,
	       (geo.base + 1 + 1) * 2,
	       (geo.base + 1 + 1 + sb.nr_imap_sects) * 2,
	       (geo.base + 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
	       (geo.base + sb.n_1st_sect) * 2);

    // ============= inode map =============
    memset(fsbuf, 0, SECTOR_SIZE);
    for(i = 0; i < (NR_CONSOLES + 2); i++)
        fsbuf[0] |= 1 << i;

    // bit 0: reserved
    // bit 1: the first inode which indicates '/'
    // bit 2: /dev_tty0
    // bit 3: /dev_tty1
    // bit 4: /dev_tty2
    assert(fsbuf[0] == 0x1F); // 0001 1111

    // write inode map to sector 2
    WR_SECT(ROOT_DEV, 2);


    // ============ sector map ===============
    memset(fsbuf, 0, SECTOR_SIZE);
    // bit 0  is reserved (sector 0),
    // NR_DEFAULT_FILE_SECTS is reserved to '/'
    uint32_t nr_sects = NR_DEFAULT_FILE_SECTS + 1; 

    for(i = 0; i < nr_sects / 8; i++)
        fsbuf[i] = 0xFF;
    
    for(j = 0; j < nr_sects % 8; j++)
        fsbuf[i] |= (1 << j); // is i not j

    // write sector map to sector n:2+nr_imap_sects
    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects);

    // zero the rest sector map
    memset(fsbuf, 0, SECTOR_SIZE);
    for(i = 1; i < sb.nr_smap_sects; i++)
        WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);
    
    // ========== inodes array ===========
    
    // inode of '/'
    memset(fsbuf, 0, SECTOR_SIZE);
    struct inode * pi = (struct inode*)fsbuf;
    pi->i_mode = I_DIRECTORY;
    pi->i_size = DIR_ENTRY_SIZE * 4; // 4 files . dev_tty0 - 2

    pi->i_start_sect = sb.n_1st_sect; // first data sect
    pi->i_nr_sects = NR_DEFAULT_FILE_SECTS;

    // inode of /dev_tty0~2
    for(i = 0; i < NR_CONSOLES; i++){
        pi = (struct inode*)(fsbuf + (INODE_SIZE * (i + 1)));
        pi->i_mode = I_CHAR_SPECIAL;
        pi->i_size = 0;
        pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
        pi->i_nr_sects = 0;
    }    

    // write inodes array sector
    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);

    // ============= '/' root directory file ============
    memset(fsbuf, 0, SECTOR_SIZE);
    struct dir_entry* pde = (struct dir_entry*)fsbuf;
    pde->inode_nr = 1;
    strcpy(pde->name, ".");

    // dir entry of /dev_tty0~2
    for(i = 0; i < NR_CONSOLES; i++){
        pde++;
        pde->inode_nr = i + 2;// dev_tty0's inode is 2
        sprintf(pde->name, "dev_tty%d", i);
    }

    // write '/' directory file into first data sector
    WR_SECT(ROOT_DEV, sb.n_1st_sect);    
}

// TODO: remove global vars
PRIVATE void init_fs(){
    int i;

    // init f_desc_table[]
    for(i = 0; i < NR_FILE_DESC; i++)
        memset(&f_desc_table[i], 0, sizeof(struct file_desc));
    
    // init inode_table[]
    for(i = 0; i < NR_INODE; i++)
        memset(&inode_table[i], 0, sizeof(struct inode));

    // init super_black[]
    struct super_block* sb = super_block;
    for(; sb < &super_block[NR_SUPER_BLOCK]; sb++)
        sb->sb_dev = NO_DEV;

    // open device - hard disk
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    mkfs();

    // load super block of ROOT
    read_super_block(ROOT_DEV);
    sb = get_super_block(ROOT_DEV);
    assert(sb->magic == MAGIC_V1);

    root_inode = get_inode(ROOT_DEV, ROOT_INODE);
}

// <ring 1>
// TODO: move pcaller out
PUBLIC void task_fs(){
    printl(">>> task fs begins. \n");
    init_fs();
    while(1){
        send_recv(RECEIVE, ANY, &fs_msg);    // TODO: replace global to local

        int src = fs_msg.source;
        pcaller = &proc_table[src]; // TODO: replace global var with function: get_proc(int)
        switch(fs_msg.type){
            case OPEN:
                fs_msg.FD = do_open();
                break;
            case CLOSE:
                printl("receive CLOSE");
                fs_msg.RETVAL = do_close();
                break;
            case READ:
            case WRITE:
                fs_msg.CNT = do_rdwt();
                break;
            default:
                dump_msg("fs: unknow message: ", &fs_msg);
                assert(0);
                break;
        }   

        // reply
        fs_msg.type = SYSCALL_RET;
        send_recv(SEND, src, &fs_msg);
    }
}
