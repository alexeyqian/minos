#include "phys_mem.h"

#define PMMGR_BLOCKS_PER_BYTE 8
#define PMMGR_BLOCK_SIZE 4096
#define PMMGR_BLOCK_ALIGN PMMGR_BLOCK_SIZE

static uint32_t _pmmgr_mem_size = 0;
static uint32_t _pmmgr_used_blocks = 0;
static uint32_t _pmmgr_max_blocks = 0;
static uint32_t* _pmmgr_mem_map = 0;

// private functions

void _pmmgr_map_set(int bit){
    _pmmgr_mem_map[bit/32] |= (1 << (bit % 32));
}

void _pmmgr_map_unset(int bit){
    _pmmgr_mem_map[bit/32] &= ~(1 << (bit % 32));
}

int _pmmgr_map_test(int bit){
    return _pmmgr_mem_map[bit/32] & (1 << (bit %32));
}

// return index of the free frame in the bit array
int _pmmgr_first_free(){
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

// finds first free "size" number of frames and returns its index
int _pmmgr_first_free_s(size_t size){
    if(size == 0) return -1;
    if(size == 1) return _pmmgr_first_free();

    for(uint32_t i = 0; i < _pmmgr_max_blocks / 32; i++){
        if(_pmmgr_mem_map[i] == 0xffffffff) continue;
        for(int j = 0; j < 32; j++){
            int bit = 1<<j;
            if(!(_pmmgr_mem_map[i] & bit)){
                int starting_bit = i *32;
                starting_bit += bit;

                uint32_t free = 0;
                for(uint32_t count = 0; count <= size; count++){
                    if(!_pmmgr_map_test(starting_bit + count))
                        free++;
                    if(free == size)
                        return i*32+j;
                }
            }
        }
    }

    return -1;
}


// public functions
void pmmgr_init(size_t mem_size, physical_addr bitmap){
    _pmmgr_mem_size = mem_size;
    _pmmgr_mem_map = (uint32_t*)bitmap;
    _pmmgr_max_blocks = mem_size * 1024 / PMMGR_BLOCK_SIZE;
    _pmmgr_used_blocks = _pmmgr_max_blocks;

    //TODO: by default, all memory are in use
    //memset(_pmmgr_mem_map, 0xf, _pmmgr_max_blocks / PMMGR_BLOCKS_PER_BYTE);
}

void pmmgr_init_region(physical_addr base, size_t size){
    int align = base / PMMGR_BLOCK_SIZE;
    int blocks = size / PMMGR_BLOCK_SIZE;
    for(; blocks >= 0; blocks--){
        _pmmgr_map_unset(align++);
        _pmmgr_used_blocks--;
    }
    //first block is always set. This insures allocs cant be 0
    _pmmgr_map_set (0);	
}

void pmmgr_uninit_region(physical_addr base, size_t size){
    int align = base / PMMGR_BLOCK_SIZE;
    int blocks = size / PMMGR_BLOCK_SIZE;
    for(; blocks >= 0; blocks--){
        _pmmgr_map_set(align++);
        _pmmgr_used_blocks++;
    }
}

void* pmmgr_alloc_block(){
    if(pmmgr_free_block_count() <= 0) return 0;
    int frame = _pmmgr_first_free();
    if(frame == -1) return 0;

    _pmmgr_map_set(frame);
    physical_addr addr = frame * PMMGR_BLOCK_SIZE;
    _pmmgr_used_blocks++;

    return (void*)addr;
}

void pmmgr_free_block(void* p){
    physical_addr addr = (physical_addr)p;
    int frame = addr / PMMGR_BLOCK_SIZE; // TODO: ??
    _pmmgr_map_unset(frame);
    _pmmgr_used_blocks--;
}

void* pmmgr_alloc_blocks(size_t size){
    if(size == 0) return 0;
    if(pmmgr_free_block_count() <= size) return 0;

    int frame = _pmmgr_first_free_s(size);
    if(frame == -1) return 0;

    for(uint32_t i = 0; i < size; i++)
        _pmmgr_map_set(frame+i);

    physical_addr addr = frame * PMMGR_BLOCK_SIZE;
    _pmmgr_used_blocks += size;

    return (void*) addr;
}

void pmmgr_free_blocks(void*p, size_t size){
    if(size == 0) return;
    
    physical_addr addr = (physical_addr)p;
    int frame = addr / PMMGR_BLOCK_SIZE;
    
    for(uint32_t i = 0; i < size; i++)
        _pmmgr_map_unset(frame + i);

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

// bool pmmgr_is_page_eanbled()
// enable_paging
// load_pdbr()
// get_pdbr()