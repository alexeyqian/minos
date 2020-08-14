#ifndef MINOS_INTERRUPT_H
#define MINOS_INTERRUPT_H

#include "ktypes.h"

void init_idt();
void irq_handler(int irq);
void put_irq_handler(int irq, pf_irq_handler_t handler);

#endif