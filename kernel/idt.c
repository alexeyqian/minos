#include "system.h"
#include "../drivers/low_level.h"

//define an idt entry
// base is 32 bit value, split in two parts
// represents the address of the entry point of ISR
// Selector is a 16 bit value and must point to a valid
// descriptor in your GDT.
struct idt_entry{
    unsigned short base_lo;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_hi;
}__attribute__((packed));

struct idt_ptr{
    unsigned short limit;
    unsigned int base;
}__attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

// exists in 'start.asm'
extern void idt_load();

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags){
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_install(){
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = &idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    // add any new ISRs to the IDT here using idt_set_gate
    
    // set CPU internal register to the new IDT
    idt_load();
}