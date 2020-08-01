#ifndef MINOS_TTY_H
#define MINOS_TTY_H

#include "const.h"
#include "types.h"
#include "ktypes.h"

#define TTY_IN_BYTES         256    // tty input queue size
#define TTY_OUT_BUF_LEN      2

#define SCROLL_SCREEN_UP	 1	    /* scroll forward */
#define SCROLL_SCREEN_DOWN	-1	    /* scroll backward */

#define SCREEN_SIZE		    (80 * 25)
#define SCREEN_WIDTH		80

typedef struct s_tty{
    // TODO: wrap fields into p_tty_buf
    // so it looks like tty_buff_s* p_tty_buff
    uint32_t  in_buf[TTY_IN_BYTES];
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
	unsigned int	current_start_addr;	
	unsigned int	original_addr;		
	unsigned int	size_in_word; // how many words does the console have
	unsigned int	cursor;			
    int is_full;
}CONSOLE;

void task_tty();
void hand_over_key_to_tty(TTY* p_tty, uint32_t key);
void tty_output_char(CONSOLE* p_con, char ch);
#endif
