
#include "const.h"
#include "ktypes.h"

// just from boot loader into kernel, 
// 1. init kernel info structure
//    get_kinfo(kinfo);
// 2. init mem_chunks
//    struct memory mem_chunks[NR_MEMS];
//    get_mem_chunks(mem_chunks);
// 3. get mem map of kernel, then then remove memory used by kernel
//    struct mem_map mem_map[NR_LOCAL_SEGS];
//    get_mem_map();

// physical memory management, reserver kenel area before allocating.
// init physical memory at kernel init

PRIVATE void get_mem_chucks(struct memory* mem_chunks){
    long base, size, limit;
    char *s, *end;
    int i, done = 0;
    struct memory *memp;

    // initialize to zero
    for(i = 0; i < NR_MEMS; i++){
        memp = &mem_chunks[i];
        memp->base = memp->size = 0;
    }

    /* The available memory is determined by MINOS' boot loader as a list of 
    * (base:size)-pairs in boothead.s. The 'memory' boot variable is set in
    * in boot.s.  The format is "b0:s0,b1:s1,b2:s2", where b0:s0 is low mem,
    * b1:s1 is mem between 1M and 16M, b2:s2 is mem above 16M. Pairs b1:s1 
    * and b2:s2 are combined if the memory is adjacent. 
    */
    //s = find_param("memory");		/* get memory boot variable */
    // TODO: hardcoded, need to be replaced later
    s = "0x100000:0x800000,0x900000:0x800000";
    for(i = 0; i < NR_MEMS && !done; i++){
        memp = &mem_chunks[i];
        base = size = 0;
        
        if(*s != 0){ // not at end yet
            //read base and expect colon as next char
            base = strtoul(s, &end, 0x10); // get number
            if(end != s && *end == ':') 
                s = ++ end; // skip :
            else
                *s = 0; // terminate, should not happen

            // read size and expect comma or assume end
            size = strtoul(s, &end, 0x10);
            if(end != s && *end == ',')
                s = ++end;
            else 
                done = 1;
        }

        limit = base + size;
        base = (base + CLICK_SIZE - 1) & ~(long)(CLICK_SIZE - 1); // next click size
        limit &= ~(long)(CLICK_SIZE - 1); // zero out CLICK_SIZE
        if(limit <= base) continue; //TODO:?
        memp->base = base >> CLICK_SHIFT;
        memp->size = (limit - base) >> CLICK_SHIFT;
    }
}