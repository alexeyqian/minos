#include "phys_mem.h"
#include "types.h"
#include "ktypes.h"

#define PMMBR_BASE             0xA00000 // physical memory manage begins from 10M
#define PMMGR_BLOCKS_PER_BYTE  8
#define PMMGR_BLOCK_SIZE       4096
#define PMMGR_BLOCK_ALIGN      PMMGR_BLOCK_SIZE

static uint32_t  _pmmgr_mem_size    = 0;
static uint32_t  _pmmgr_max_blocks  = 0;
static uint32_t  _pmmgr_used_blocks = 0;
static uint32_t* _pmmgr_mem_map     = 0;

PRIVATE void pmmgr_map_set(int bit){
    _pmmgr_mem_map[bit/32] |= (1 << (bit % 32));
}

PRIVATE void pmmgr_map_unset(int bit){
    _pmmgr_mem_map[bit/32] &= ~(1 << (bit % 32));
}

int pmmgr_map_test(int bit){
    return _pmmgr_mem_map[bit/32] & (1 << (bit %32));
}

/* 
 * @return on success: index of the free frame in the bit array, on fail: -1
 **/
PRIVATE int pmmgr_first_free(){
    for(uint32_t i = 0; i < _pmmgr_max_blocks / 32; i++){
        if(_pmmgr_mem_map[i] == 0xffffffff) continue;
        for(int j = 0; j < 32; j++){
            int bit = 1 << j;
            if(!(_pmmgr_mem_map[i] & bit))
                return i * 32 + j;
        }
    }
    return -1;
}

/* 
 * @return first free "size" number of frames and returns its index, on fail: -1
 * */
int pmmgr_first_free_s(size_t size){
    if(size == 0) return -1;
    if(size == 1) return pmmgr_first_free();

    for(uint32_t i = 0; i < _pmmgr_max_blocks / 32; i++){
        if(_pmmgr_mem_map[i] == 0xffffffff) continue;
        for(int j = 0; j < 32; j++){
            int bit = 1<<j;
            if(!(_pmmgr_mem_map[i] & bit)){
                int starting_bit = i *32;
                starting_bit += bit;

                uint32_t free = 0;
                for(uint32_t count = 0; count <= size; count++){
                    if(!pmmgr_map_test(starting_bit + count))
                        free++;
                    if(free == size)
                        return i*32+j;
                }
            }
        }
    }

    return -1;
}

void pmmgr_init_region(physical_addr base, size_t size){
    int align = base / PMMGR_BLOCK_SIZE;
    int blocks = size / PMMGR_BLOCK_SIZE;
    for(; blocks >= 0; blocks--){
        pmmgr_map_unset(align++);
        _pmmgr_used_blocks--;
    }
    //first block is always set. This insures allocs cant be 0
    pmmgr_map_set (0);	// 0> position/index = 0
}

void pmmgr_uninit_region(physical_addr base, size_t size){
    int align = base / PMMGR_BLOCK_SIZE;
    int blocks = size / PMMGR_BLOCK_SIZE;
    for(; blocks >= 0; blocks--){
        pmmgr_map_set(align++);
        _pmmgr_used_blocks++;
    }
}

void* pmmgr_alloc_block(){
    if(pmmgr_free_block_count() <= 0) return 0;
    int frame = pmmgr_first_free();
    if(frame == -1) return 0;

    pmmgr_map_set(frame);
    physical_addr addr = frame * PMMGR_BLOCK_SIZE;
    _pmmgr_used_blocks++;

    return (void*)addr;
}

void pmmgr_free_block(void* p){
    physical_addr addr = (physical_addr)p;
    int frame = addr / PMMGR_BLOCK_SIZE; 
    pmmgr_map_unset(frame);
    _pmmgr_used_blocks--;
}

void* pmmgr_alloc_blocks(size_t size){
    if(size == 0) return 0;
    if(pmmgr_free_block_count() <= size) return 0;

    int frame = pmmgr_first_free_s(size);
    if(frame == -1) return 0;

    for(uint32_t i = 0; i < size; i++)
        pmmgr_map_set(frame+i);

    physical_addr addr = frame * PMMGR_BLOCK_SIZE;
    _pmmgr_used_blocks += size;

    return (void*) addr;
}

void pmmgr_free_blocks(void*p, size_t size){
    if(size == 0) return;
    
    physical_addr addr = (physical_addr)p;
    int frame = addr / PMMGR_BLOCK_SIZE;
    
    for(uint32_t i = 0; i < size; i++)
        pmmgr_map_unset(frame + i);

    _pmmgr_used_blocks -= size;
}

size_t pmmgr_get_mem_size(){
    return _pmmgr_mem_size;
}

uint32_t pmmgr_max_block_count(){
    return _pmmgr_max_blocks;
}

uint32_t pmmgr_free_block_count(){
    return _pmmgr_max_blocks - _pmmgr_used_blocks;
}

uint32_t pmmgr_block_size(){
    return PMMGR_BLOCK_SIZE;
}

/*
 * physical mem managment starts from 10M at 0xa00000
 * */
PUBLIC void pmmgr_init(struct boot_params* pbp){	
    _pmmgr_mem_size = pbp->mem_size;    
    _pmmgr_max_blocks = _pmmgr_mem_size / PMMGR_BLOCK_SIZE;
    _pmmgr_used_blocks = _pmmgr_max_blocks;
    _pmmgr_mem_map = (uint32_t*)PMMBR_BASE;
    
	for(int i = 0; i < pbp->mem_range_count; i++){
		if(pbp->mem_ranges[i].type == 1)
			pmmgr_init_region(pbp->mem_ranges[i].baseaddr_low, pbp->mem_ranges[i].length_low);			
	}
	// Reserver lower 10M
	pmmgr_uninit_region(0x0, PMMBR_BASE); 
    // reserve memory for mem bitmap itself
    //memset(_pmmgr_mem_map, 0xf, _pmmgr_max_blocks / PMMGR_BLOCKS_PER_BYTE);	
    uint32_t bitmap_size = (pbp->mem_size - (uint32_t)PMMBR_BASE) / (PMMGR_BLOCK_SIZE * PMMGR_BLOCKS_PER_BYTE);
    pmmgr_uninit_region(_pmmgr_mem_map, bitmap_size);

    kprintf(">>> pmmgr max blocks %d, used blocks %d, bitmap size: %d bytes\n", _pmmgr_max_blocks, _pmmgr_used_blocks, bitmap_size);
	
	uint32_t* p1 = (uint32_t*) pmmgr_alloc_block();
	kprintf(">>> test p1 allocated at: 0x%x", p1);
	
	uint32_t* p2 = (uint32_t*)pmmgr_alloc_block();
	kprintf(">>> test p2 allocated at: 0x%x", p2);

	pmmgr_free_block(p1);
	pmmgr_free_block(p2);
}
