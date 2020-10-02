#ifndef _TYPES_H
#define _TYPES_H

typedef unsigned int size_t;
typedef int ssize_t;
typedef long time_t;
typedef long clock_t;
typedef long useconds_t; // time in microseconds

// types used in disk, inode, etc.
typedef short dev_t;             // holds major/minor device pair
typedef char  gid_t;             // group id
typedef unsigned long ino_t;     // i-node number
typedef unsigned short mode_t;   // file type and permission bits
typedef short nlink_t;           // number of links to a file
typedef unsigned long off_t;     // offset within a file
typedef int pid_t;               // process id (must be signed)
typedef short uid_t;             // user id
typedef unsigned long zone_t;    // zone number
typedef unsigned long block_t;   // block number
typedef unsigned long bit_t;     // bit number in a bit map
typedef unsigned short bitchunk_t; // collection of bits in a bitmap

typedef unsigned char   u8_t;	   
typedef unsigned short u16_t;	   
typedef unsigned long  u32_t;	   

typedef char            i8_t;      
typedef short          i16_t;      
typedef long           i32_t;      
typedef struct { u32_t _[2]; } u64_t;

typedef int              bool_t;

typedef char             int8_t;
typedef short            int16_t;
typedef int              int32_t;

typedef unsigned char    uint8_t;
typedef unsigned short   uint16_t;
typedef unsigned int     uint32_t;
typedef unsigned long long uint64_t;

typedef int32_t          intptr_t;
typedef uint32_t         uintptr_t;

typedef char* va_list;
typedef void (*sighandler_t)(int);

#endif