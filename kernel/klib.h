#ifndef _MINOS_KLIB_H_
#define _MINOS_KLIB_H_

#include "types.h"

// IO read and write
// in: read data from device register, out: write data to device register
// I/O addresses/ports that are mapped to speciﬁc controller registers

uint8_t in_byte(io_port_t port);
void out_byte(io_port_t port, uint8_t data);
uint16_t in_word(io_port_t port);
void out_word(io_port_t, uint16_t data);

void kmemcpy(void* dest, void* src, int size);
void kprint_str(char* msg);
void kprint_int_as_hex(int irq);
char *itoa(char* buf, int num);

#endif