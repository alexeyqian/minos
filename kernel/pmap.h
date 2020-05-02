#ifndef PMAP_H
#define PMAP_H

struct page_info{
    //next page on the free list.
    struct page_info *pp_link;
    // pp_ref is the count of pointers to this page, 
    // usally in page table entries.
    // virtual memory allows you to map two different virtual pages to the 
    // same physical page, known as a memory mapping.
    // if the count hits 0, the physical page is considered not-in-use.
    uint16_t pp_ref;
};

#endif