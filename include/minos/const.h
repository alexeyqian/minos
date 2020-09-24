#ifndef MINOS_CONST_H
#define MINOS_CONST_H

#ifndef CHIP
#error CHIP is not defined
#endif

#define EXTERN  extern
#define PRIVATE static
#define PUBLIC 

#define TRUE  1
#define FALSE 0

#define HZ 60 // clock frequency

#define SUPER_USER (uid_t)0  

// devices 0377 = b000 011 111 111
#define MAJOR    8 // (dev>>MAJOR) & 0377
#define MINOR    0 // (dev>>MAJOR) & 0377  

#define NULL ((void*)0) // null pointer

#define PROC_NAME_LEN  16

#define MAX(A, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#if (CHIP == INTEL)
#define CLICK_SIZE 1024
#define CLICK_SHIFT 10
#endif

/* Flag bits for i_mode in the inode. */
#define I_TYPE          0170000	/* this field gives inode type */
#define I_REGULAR       0100000	/* regular file, not dir or special */
#define I_BLOCK_SPECIAL 0060000	/* block special file */
#define I_DIRECTORY     0040000	/* file is a directory */
#define I_CHAR_SPECIAL  0020000	/* character special file */
#define I_NAMED_PIPE	0010000 /* named pipe (FIFO) */
#define I_SET_UID_BIT   0004000	/* set effective uid_t on exec */
#define I_SET_GID_BIT   0002000	/* set effective gid_t on exec */
#define ALL_MODES       0006777	/* all bits for user, group and others */
#define RWX_MODES       0000777	/* mode bits for RWX only */
#define R_BIT           0000004	/* Rwx protection bit */
#define W_BIT           0000002	/* rWx protection bit */
#define X_BIT           0000001	/* rwX protection bit */
#define I_NOT_ALLOC     0000000	/* this inode is free */

/* Flag used only in flags argument of dev_open. */
#define RO_BIT		0200000	/* Open device readonly; fail if writable. */

/* Some limits. */
#define MAX_BLOCK_NR  ((block_t) 077777777)	/* largest block number */
#define HIGHEST_ZONE   ((zone_t) 077777777)	/* largest zone number */
#define MAX_INODE_NR ((ino_t) 037777777777)	/* largest inode number */
#define MAX_FILE_POS ((off_t) 037777777777)	/* largest legal file offset */

#define NO_BLOCK              ((block_t) 0)	/* absence of a block number */
#define NO_ENTRY                ((ino_t) 0)	/* absence of a dir entry */
#define NO_ZONE                ((zone_t) 0)	/* absence of a zone number */
#define NO_DEV                  ((dev_t) 0)	/* absence of a device numb */

#endif