#ifndef LOW_LEVEL_H
#define LOW_LEVEL_H

// in: read, out: write
// I/O addresses/ports that are mapped to speciﬁc controller registers
// return byte from IO port
unsigned char port_byte_in(unsigned short port);
// set IO port to data
void port_byte_out(unsigned short port, unsigned char data);
unsigned short port_word_in(unsigned short port);
void port_word_out(unsigned short port, unsigned short data);

#endif