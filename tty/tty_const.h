#ifndef MINOS_TTY_CONST_H
#define MINOS_TTY_CONST_H

#define CRTC_ADDR_REG			    0x3D4	/* CRT Controller Registers - Address Register */
#define CRTC_DATA_REG			    0x3D5	/* CRT Controller Registers - Data Registers */
#define CRTC_DATA_IDX_START_ADDR_H	0xC	    /* register index of video mem start address (MSB) */
#define CRTC_DATA_IDX_START_ADDR_L	0xD  	/* register index of video mem start address (LSB) */
#define CRTC_DATA_IDX_CURSOR_H		0xE  	/* register index of cursor position (MSB) */
#define CRTC_DATA_IDX_CURSOR_L		0xF  	/* register index of cursor position (LSB) */
#define V_MEM_BASE			        0xB8000	/* base of color video memory */
#define V_MEM_SIZE			        0x8000	/* 32K: B8000H -> BFFFFH */

#endif