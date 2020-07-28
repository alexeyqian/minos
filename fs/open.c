#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "string.h"
#include "klib.h"
#include "ipc.h"

// open/create a file
// flag: O_CREATE, O_RDWR
// return file descriptor if successful, otherwise -1
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

// return inode nr
PRIVATE int alloc_imap_bit(int dev){
    
}

// create a file and return it's inode ptr
// return 0 if failed
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

PUBLIC int do_close(){
    int fd = fs_msg.FD;
    put_inode(pcaller->filp[fd]->fd_inode);
    pcaller->filp[fd]->fd_inode = 0;
    pcaller->filp[fd] = 0;

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