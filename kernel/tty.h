#ifndef MINOS_TTY_H
#define MINOS_TTY_H

#include "const.h"
#include "types.h"
#include "ktypes.h"

#define TTY_IN_BYTES         256    // tty input queue size

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

    struct s_console* p_console;
} TTY;

typedef struct s_console // CONSOLE is a video memory region
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置   */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
}CONSOLE;

void task_tty();
void hand_over_key_to_tty(TTY* p_tty, uint32_t key);
void tty_output_char(CONSOLE* p_con, char ch);
int sys_printx(int _unused1, int _unused2, char* s, struct proc* p_proc);
int sys_write(char* buf, int len, struct proc* p_proc);
#endif
