#ifndef I_MINOS_TYPE_H_
#define I_MINOS_TYPE_H_

typedef unsigned int  vir_clicks;
typedef unsigned int  vir_bytes;
typedef unsigned int  phys_clicks;
typedef unsigned long phys_bytes;

// memory map for local text, statck, data segments.
struct mem_map{
    vir_clicks mem_vir;
    vir_clicks mem_len;
    phys_clicks mem_phys;
};

struct far_mem{
    int in_use;
    phys_clicks mem_phys;
    vir_clicks  mem_len;
};

struct vir_addr{
    int proc_nr;
    int segment;
    vir_bytes offset;
};

struct kinfo{
    phys_bytes code_base; // base of kernel code
    phys_bytes code_size;
    phys_bytes data_base;
    phys_bytes data_size;
    
    vir_bytes  proc_addr; // virtual address of process table

    int nr_procs; // number of user processes
    int nr_tasks; // number of kernel tasks
    char release[6];
    char version[6];
};

#endif