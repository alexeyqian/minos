#include "pmmgr.h"

#define PMMGR_BLOCKS_PER_BYTE 8
#define PMMGR_BLOCK_SIZE 4096
#define PMMGR_BLOCK_ALIGN PMMGR_BLOCK_SIZE

// size of physical memory
static uint32_t _pmmgr_memory_size = 0;
static uint32_t _pmmgr_used_blocks = 0;
static uint32_t _pmmgr_max_blocks = 0;
// memory map bit array, each bit represents a memory block
static uint32_t* _pmmgr_memory_map = 0;

// treat memory map as an array of bits rather than an array of ints.
// The bit is a value from 0...x, where x is the bit that we want to set in the memory map. 
// We divide the bit by 32 to get the integer index in _pmmgr_memory_map that the bit is in.
inline void pmmgr_map_set(int bit){
    _pmmgr_memory_map[bit / 32] |= (1 << (bit % 32));
}

inline void pmmgr_map_unset(int bit){
    _pmmgr_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

inline bool pmmgr_map_test(int bit){
    return _pmmgr_memory_map[bit / 32] & (1 << (bit % 32));
}

uint32_t pmmgr_get_memory_size(){
    return _pmmgr_memory_size;    
}

// TODO
// return max number of memory blocks in system
int pmmgr_get_max_block_count(){
    _pmmgr_memory_size / PMMGR_BLOCK_SIZE;
}

int pmmgr_map_first_free(){
    for(int i = 0; i < pmmgr_get_max_block_count() / 32; i++){
        if(_pmmgr_memory_map[i] != 0xffffffff){
            for(int j = 0; j < 32; j++){
                int bit = 1 << j;
                if(!(_pmmgr_memory_map[i] & bit))
                    return i*32+j;
            }
        }
    }

    return -1;
}

// TODO
// returns the index of the first free series of frames of a specific size
int  pmmgr_map_first_free_s(uint32_t size){
    return -1;
}



// We do not know what areas of memory are safe to work with, only the kernel does. 
// Because of this, by default all of memory is in use. 
// The kernel abtains the memory map from the kernel and uses this 
// routine to initialize available regions of memory that we can use.
void pmmgr_init_region(uint32_t base, uint32_t size){
    int align = base / PMMGR_BLOCK_SIZE;
    int blocks = size / PMMGR_BLOCK_SIZE;

    for(; blocks > 0; blocks--){
        pmmgr_map_unset(align++);
        _pmmgr_used_blocks--;
    }

    // the first block is always set.
    // This insures us that the PMM can return null pointers for allocation errors. 
    // This also insures that any data structures defined within the first 64 KB of memory 
    // are not overwritten or touched, including the Interrupt Vector Table (IVT) and Bios Data Area (BDA).
    pmmgr_map_set(0); // ??
}

void pmmgr_deinit_region(uint32_t base, uint32_t size){
    int align = base / PMMGR_BLOCK_SIZE;
    int blocks = size / PMMGR_BLOCK_SIZE;

    for(; blocks > 0; blocks --){
        pmmgr_map_unset(align++);
        _pmmgr_used_blocks++;
    }
}

void* pmmgr_alloc_block(){
    if(pmmgr_get_free_block_count() <= 0)
        return 0; // out of memory
    
    int frame = pmmgr_map_first_free();
    if(frame == -1) 
        return 0; // out of memory

    pmmgr_map_set(frame);
    _pmmgr_used_blocks++;
    uint32_t addr = frame * PMMGR_BLOCK_SIZE;
    return (void*)addr;
}

// TODO
void* pmmgr_alloc_blocks(uint32_t blocks){

}

void pmmgr_free_block(void* p){
    uint32_t addr = (uint32_t)p;
    int frame = addr / PMMGR_BLOCK_SIZE;
    pmmgr_map_unset(frame);
    _pmmgr_used_blocks--;
}

void pmmgr_init(uint32_t mem_size_kb, uint32_t bitmap){
    _pmmgr_memory_size = mem_size_kb;
    _pmmgr_memory_map = bitmap;
    _pmmgr_max_blocks = pmmgr_get_memory_size() * 1024 / PMMGR_BLOCK_SIZE;
    _pmmgr_used_blocks = pmmgr_get_max_block_count();

    // by default, all of memory is in use
    memset(_pmmgr_memory_map, 0xff, pmmgr_get_max_block_count() / PMMGR_BLOCKS_PER_BYTE);
}