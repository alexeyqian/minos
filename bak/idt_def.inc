; TODO: below lines should be move to .c file as inline asm
extern idtp
global idt_load
idt_load:
    lidt [idtp]
    ret