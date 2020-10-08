#ifndef MINOS_TTY_H
#define MINOS_TTY_H

#include <const.h>
#include <sys/types.h>

#define CRTC_ADDR_REG			    0x3D4	/* CRT Controller Registers - Address Register */
#define CRTC_DATA_REG			    0x3D5	/* CRT Controller Registers - Data Registers */
#define CRTC_DATA_IDX_START_ADDR_H	0xC	    /* register index of video mem start address (MSB) */
#define CRTC_DATA_IDX_START_ADDR_L	0xD  	/* register index of video mem start address (LSB) */
#define CRTC_DATA_IDX_CURSOR_H		0xE  	/* register index of cursor position (MSB) */
#define CRTC_DATA_IDX_CURSOR_L		0xF  	/* register index of cursor position (LSB) */

#define TTY_IN_BYTES         256    // tty input queue size
#define TTY_OUT_BUF_LEN      2

#define SCROLL_SCREEN_UP	 1	    /* scroll forward */
#define SCROLL_SCREEN_DOWN	-1	    /* scroll backward */

#define SCREEN_SIZE		    (80 * 25)
#define SCREEN_WIDTH		80

typedef struct s_tty{
    uint32_t  in_buf[TTY_IN_BYTES];   // TODO: rename to inbuf
    int       inbuf_count;            // how many keys are in buffer
    uint32_t* p_inbuf_head;           // point to next available space
    uint32_t* p_inbuf_tail;           // point to next should be processed value    

    int tty_caller;    // usally is task_fs
    int tty_procnr;    // process p who request data
    void* tty_req_buf; // process P's buf address for putting data
    int tty_left_cnt;  // how much requested
    int tty_trans_cnt; // how much have been transferred.

    struct s_console* p_console;
} TTY;

typedef struct s_console // CONSOLE is a video memory region
{
	unsigned int	current_start_addr;	// TODO: should use phy_addr type
	unsigned int	original_addr;		
	unsigned int	size_in_word; // how many words does the console have
	unsigned int	cursor;			
    int is_full;
} CONSOLE;

void svc_tty();
void clear_screen(int pos, int len);
void tty_output_char(CONSOLE* p_con, char ch);
//void tty_reset_start_addr();

#endif