#include "phys_mem.h"
#include "types.h"
#include "ktypes.h"
#include "screen.h"

#define ALLOC_BASE             0x300000 // physical memory manage begins from 4M
#define PMMGR_BLOCKS_PER_BYTE  8
#define PMMGR_BLOCK_SIZE       4096
#define PMMGR_BLOCK_ALIGN      PMMGR_BLOCK_SIZE
#define PMMGR_UINT32_SIZE      32 // sizeof(uint32_t)

PRIVATE uint32_t  _mem_size    = 0; // in kb or bytes
PRIVATE uint32_t  _max_blocks  = 0;
PRIVATE uint32_t  _used_blocks = 0;
PRIVATE uint32_t* _mem_map_ptr = 0;

PRIVATE void pmmgr_map_set(uint32_t bit_index){
    _mem_map_ptr[bit_index/PMMGR_UINT32_SIZE] |= (uint32_t)1 << (bit_index % PMMGR_UINT32_SIZE);
}

PRIVATE void pmmgr_map_unset(uint32_t bit_index){
    _mem_map_ptr[bit_index/PMMGR_UINT32_SIZE] &= ~((uint32_t)1 << (bit_index % PMMGR_UINT32_SIZE));
}

PRIVATE bool_t pmmgr_map_test(uint32_t bit_index){
    return (bool_t)(_mem_map_ptr[bit_index/PMMGR_UINT32_SIZE] & (1 << (bit_index % PMMGR_UINT32_SIZE)));
}

/* 
 * @return on success: index of the free frame in the bit array, on fail: -1
 **/
PRIVATE int pmmgr_first_free(){
    for(uint32_t i = 0; i < _max_blocks / 32; i++){
        if(_mem_map_ptr[i] == 0xffffffff) 
            continue;

        for(uint32_t j = 0; j < PMMGR_UINT32_SIZE; j++){
            uint32_t bit = 1 << j;
            if(!(_mem_map_ptr[i] & bit)) 
                return (int)(i * PMMGR_UINT32_SIZE + j); // saft conv
        }
    }
    return -1;
}

/* 
 * @return first free "size" number of frames and returns its index, on fail: -1
 * */
PRIVATE int pmmgr_first_free_s(size_t size){
    if(size == 0) return -1;
    if(size == 1) return pmmgr_first_free();

    for(uint32_t i = 0; i < _max_blocks / PMMGR_UINT32_SIZE; i++){
        if(_mem_map_ptr[i] == 0xffffffff) continue;
        for(uint32_t j = 0; j < PMMGR_UINT32_SIZE; j++){
            uint32_t bit = 1<<j;
            if(!(_mem_map_ptr[i] & bit)){
                uint32_t starting_bit = i * PMMGR_UINT32_SIZE;
                starting_bit += bit;

                uint32_t free = 0;
                for(uint32_t count = 0; count <= size; count++){
                    if(!pmmgr_map_test(starting_bit + count))
                        free++;
                    if(free == size)
                        return (int)(i * PMMGR_UINT32_SIZE + j);
                }
            }
        }
    }

    return -1;
}

/* not used
PRIVATE void pmmgr_init_region(physical_addr base, size_t size){
    uint32_t align = base / PMMGR_BLOCK_SIZE;
    int blocks = size / PMMGR_BLOCK_SIZE;
    for(; blocks >= 0; blocks--){
        pmmgr_map_unset(align++);
        _used_blocks--;
    }
    //first block is always set. This insures first block (block index = 0) is always reserved as a safe guard
    uint32_t bit_index = 0;
    pmmgr_map_set (bit_index);
}
*/

PRIVATE void pmmgr_uninit_region(physical_addr base, size_t size){
    uint32_t align = base / PMMGR_BLOCK_SIZE;
    int blocks = (int)size / PMMGR_BLOCK_SIZE;
    for(; blocks >= 0; blocks--){
        pmmgr_map_set(align++);
        _used_blocks++;
    }
}

// block 0 is reserved
PUBLIC void* pmmgr_alloc_block(){
    if(pmmgr_free_block_count() <= 0) return NULL;
    int frame = pmmgr_first_free();
    if(frame <= 0) return NULL;
    // assert(frame > 0)
    pmmgr_map_set((uint32_t)frame);
    physical_addr addr = (uint32_t)frame * PMMGR_BLOCK_SIZE;
    _used_blocks++;

    return (void*)addr;
}

PUBLIC void pmmgr_free_block(void* p){
    physical_addr addr = (physical_addr)p;
    uint32_t frame = addr / PMMGR_BLOCK_SIZE; 
    pmmgr_map_unset(frame);
    _used_blocks--;
}

PUBLIC void* pmmgr_alloc_blocks(size_t size){
    if(size == 0) return NULL;
    if(pmmgr_free_block_count() <= size) return NULL;

    int frame = pmmgr_first_free_s(size);
    if(frame <= 0) return NULL;

    for(uint32_t i = 0; i < size; i++)
        pmmgr_map_set((uint32_t)frame + i);

    physical_addr addr = (uint32_t)frame * PMMGR_BLOCK_SIZE;
    _used_blocks += size;

    return (void*) addr;
}

PUBLIC void pmmgr_free_blocks(void*p, size_t size){
    if(size == 0) return;
    
    physical_addr addr = (physical_addr)p;
    uint32_t frame = addr / PMMGR_BLOCK_SIZE;
    
    for(uint32_t i = 0; i < size; i++)
        pmmgr_map_unset(frame + i);

    _used_blocks -= size;
}

size_t pmmgr_get_mem_size(){
    return _mem_size;
}

uint32_t pmmgr_max_block_count(){
    return _max_blocks;
}

uint32_t pmmgr_free_block_count(){
    return _max_blocks - _used_blocks;
}

uint32_t pmmgr_block_size(){
    return PMMGR_BLOCK_SIZE;
}

/*
 * physical mem managment starts from 10M at 0xa00000
 * */
PUBLIC void pmmgr_init(struct boot_params* pbp){	
    _mem_size = pbp->mem_size;    
    _max_blocks = _mem_size / PMMGR_BLOCK_SIZE;
    _used_blocks = 0; //_max_blocks;            
    _mem_map_ptr = (uint32_t *)ALLOC_BASE; // init pointer to an address

    /* TODO: ignored for now
	for(utint32_t i = 0; i < pbp->mem_range_count; i++){
		if(pbp->mem_ranges[i].type == 1)
			pmmgr_init_region(pbp->mem_ranges[i].baseaddr_low, pbp->mem_ranges[i].length_low);			
	}*/

	// Reserver 0 - 4M memory
	pmmgr_uninit_region(0x0, ALLOC_BASE); 
    // reserve memory for mem bitmap    
    kassert(_mem_size > ALLOC_BASE);
    uint32_t bitmap_size = (_mem_size - ALLOC_BASE) / (PMMGR_BLOCK_SIZE * PMMGR_BLOCKS_PER_BYTE);
    pmmgr_uninit_region(ALLOC_BASE, bitmap_size);

    kprintf(">>> pmmgr max blocks %d, used blocks %d, bitmap size: %d bytes\n", _max_blocks, _used_blocks, bitmap_size);
	
	void* p1 = pmmgr_alloc_block();
	kprintf(">>> test p1 allocated at: 0x%x", p1);
	
	void* p2 = pmmgr_alloc_block();
	kprintf(">>> test p2 allocated at: 0x%x", p2);

	pmmgr_free_block(p1);
	pmmgr_free_block(p2);
}
