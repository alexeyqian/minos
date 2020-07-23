#ifndef MINOS_INTERRUPT_H
#define MINOS_INTERRUPT_H

void init_idt();
void irq_handler(int irq);

#endif