#ifndef _TTY_TTY_H
#define _TTY_TTY_H

// POSIX headers
#include <const.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <utils.h>

// MINOS specific headers
#include <minos/const.h>
#include <minos/types.h>
#include <minos/proto.h>
#include <minos/keyboard.h>
#include <minos/tty.h>

#define V_MEM_BASE			        0xB8000	/* base of color video memory */
#define V_MEM_SIZE			        0x8000	/* 32K: B8000H -> BFFFFH */


/*
 * e.g.	MAKE_COLOR(BLUE, RED)
 *	MAKE_COLOR(BLACK, RED) | BRIGHT
 *	MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH
 */
#define	BLACK	0x0 	/* 0000 */
#define	WHITE	0x7 	/* 0111 */
#define	RED	    0x4 	/* 0100 */
#define	GREEN	0x2 	/* 0010 */
#define	BLUE	0x1 	/* 0001 */
#define	FLASH	0x80	/* 1000 0000 */
#define	BRIGHT	0x08	/* 0000 1000 */
#define	MAKE_COLOR(x,y)	((x<<4) | y)	/* MAKE_COLOR(Background,Foreground) */

#define DEFAULT_CHAR_COLOR	(MAKE_COLOR(BLACK, WHITE))
#define GRAY_CHAR		(MAKE_COLOR(BLACK, BLACK) | BRIGHT)
#define RED_CHAR		(MAKE_COLOR(BLUE, RED) | BRIGHT)

#endif