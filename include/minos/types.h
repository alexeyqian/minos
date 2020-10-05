#ifndef MINOS_TYPES_H
#define MINOS_TYPES_H

#include <sys/types.h>

struct mess1{
    int m1i1;
    int m1i2;
    int m1i3;
    int m1i4;
};

struct mess2{
    void* m2p1;
    void* m2p2;
    void* m2p3;
    void* m2p4;
};

struct mess3{
    int m3i1;
    int m3i2;
    int m3i3;
    int m3i4;
    uint64_t m3l1; 
    uint64_t m3l2; 
    void* m3p1;
    void* m3p2;
};

enum kcall_type{
    KC_RET,
	KC_IN_BYTE,
	KC_OUT_BYTE,
    KC_PORT_READ,
    KC_PORT_WRITE,
    KC_ENABLE_IRQ,
    KC_DISABLE_IRQ,
	KC_ENABLE_INT,
	KC_DISABLE_INT,
    KC_KEYBOARD_READ,
	KC_PUTS,
    KC_TICKS
};

typedef struct kcall_params{
    int type;
    union{
        struct mess1 m1;
        struct mess2 m2;
        struct mess3 m3;
    }u;
} KCALL_PARAMS;

struct two_ints_s{
    int i1;
    int i2;
};

struct three_ints_s{
    int i1;
    int i2;
    int i3;
};

enum kmessage_type{ 
	HARD_INT = 1,
	// sys task
	GET_TICKS, GET_PID, GET_RTC_TIME,
	// fs
	OPEN, CLOSE, READ, WRITE, LSEEK, STAT, UNLINK,
	// fs & tty
	SUSPEND_PROC, RESUME_PROC,
	// MM
	EXEC, WAIT, 
	// fs & mm
	FORK, EXIT,
	// tty, sys, fs, mm, etc
	SYSCALL_RET, // rename to KMESSAGE_RET
    /* message type for drivers */
	DEV_OPEN = 1001,
	DEV_CLOSE,
	DEV_READ,
	DEV_WRITE,
	DEV_IOCTL,
	// for debug
	DISK_LOG
};

// cannot use pointer inside, since message might be transfered 
// between different rings, and can only be transferred by value instead of reference.
typedef struct kmessage{ 
    int source;
    int type;
    union{
        struct mess1 m1;
        struct mess2 m2;
        struct mess3 m3;
    }u;
}KMESSAGE; 

// TODO: add prefix MSG_FIELD_
#define	PID		    u.m3.m3i2
#define	STATUS		u.m3.m3i1
#define	RETVAL		u.m3.m3i1
#define	PROC_NR		u.m3.m3i3
#define	BUF	    	u.m3.m3p2
#define BUF_LEN     u.m3.m3i3

#define	DEVICE		u.m3.m3i4
#define	POSITION	u.m3.m3l1
#define	OFFSET		u.m3.m3i2 
#define	WHENCE		u.m3.m3i3 

#define	FD		    u.m3.m3i1 
#define	PATHNAME	u.m3.m3p1 

#define	FLAGS		u.m3.m3i1 
#define	NAME_LEN	u.m3.m3i2 
#define	CNT		    u.m3.m3i2
#define	REQUEST		u.m3.m3i2

struct dev_drv_map{
    int driver_nr;
};

#endif