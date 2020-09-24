#ifndef MINOS_TYPE_H
#define MINOS_TYPE_H

typedef unsigned int  vir_bytes;   //TODO: long? virtual addr/length in bytes
typedef unsigned long phys_bytes;  // physical addr/length in bytes
typedef unsigned int  vir_clicks;  // virtual addr/length in clicks
typedef unsigned int  phys_clicks; // physical addr/length in clicks

// memory map for local text, data and stack
struct mem_map{
    vir_clicks  mem_vir;   // virtual address
    phys_clicks mem_phys;  // physical address
    vir_clicks  mem_len;   // length
};

// memory map for remote memory areas, such as RAM disk
struct far_mem{
    int in_use; // zero means not in use
    phys_clicks mem_phys;
    vir_clicks  mem_len;
};

struct vir_addr{ // TODO: rename to proc_vir_addr
    int proc_nr;
    int segment;
    vir_bytes offset;
};

struct vir_cp_req{
    struct vir_addr src;
    struct vir_addr dst;
    phys_bytes count;
};

// PM passes the address of a structure of this type to KERNEL 
// when sys_sendsig() is invoked as part of the signal catching machanism.
// The structure contains all the infomation that KERNEL needs to build the signal stack.
struct sigmsg{
    int sm_signo;
    unsigned long sm_mask; // mask to restore when handler returns
    vir_bytes sm_sighandler; // addr of handler
    vir_bytes sm_sigreturn; // addr of _sigreturn in C library
    vir_bytes sm_stkptr; // user stack pointer
};


/* This is used to obtain system information through SYS_GETINFO. */
struct kinfo {
  phys_bytes code_base;		/* base of kernel code */
  phys_bytes code_size;		
  phys_bytes data_base;		/* base of kernel data */
  phys_bytes data_size;
  vir_bytes  proc_addr;		/* virtual address of process table */
  phys_bytes kmem_base;		/* kernel memory layout (/dev/kmem) */
  phys_bytes kmem_size;
  phys_bytes bootdev_base;	/* boot device from boot image (/dev/boot) */
  phys_bytes bootdev_size;
  phys_bytes bootdev_mem;
  phys_bytes params_base;	/* parameters passed by boot loader */
  phys_bytes params_size;
  int nr_procs;			/* number of user processes */
  int nr_tasks;			/* number of kernel tasks */
  char release[6];		/* kernel release number */
  char version[6];		/* kernel version number */
  int relocking;		/* relocking check (for debugging) */
};

struct machine {
  int pc_at;
  int ps_mca;
  int processor;
  int protected;
  int vdu_ega;
  int vdu_vga;
};
#endif