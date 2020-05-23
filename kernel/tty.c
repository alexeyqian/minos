#include "const.h"
#include "types.h"
#include "asm_util.h"
#include "klib.h"
#include "keyboard.h"
#include "tty.h"

// tty = keyboard (kb buffer) + console (screen buffer)
extern TTY     tty_table[];
extern CONSOLE console_table[];
extern int nr_current_console;
#define TTY_FIRST (tty_table)
#define TTY_END   (tty_table + NR_CONSOLES)

void init_tty(TTY* p_tty){
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
    int nr_tty = p_tty - tty_table;
    p_tty->p_console = console_table + nr_tty;
}

bool_t is_current_console(CONSOLE* p_con){
    return (p_con == &console_table[nr_current_console]);
}

static void tty_do_read(TTY* p_tty){
    if(is_current_console(p_tty->p_console))
        keyboard_read(p_tty);
}

void out_char(CONSOLE* p_con, char ch){
    int display_position = get_cursor();
    uint8_t* p_vmem = (uint8_t*)(V_MEM_BASE + display_position);
    *p_vmem++ = ch;
    *p_vmem++ = DEFAULT_CHAR_COLOR;
    // disp_pos + 2;
    // set_cursor(disp_pos/2);    
}

/*
static void set_cursor(unsigned int position){
    disable_int();
    //...

    enable_int();
}*/

static void tty_do_write(TTY* p_tty){
    if(p_tty->inbuf_count){
        char ch = *(p_tty->p_inbuf_tail);
        p_tty->p_inbuf_tail++;
        if(p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES)
            p_tty->p_inbuf_tail = p_tty->in_buf;
        p_tty->inbuf_count--;
        out_char(p_tty->p_console, ch);
    }
}

void task_tty(){
    TTY* p_tty;

    init_keyboard(); // TODO: ?? should only init once
    for(p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
        init_tty(p_tty);

    nr_current_console = 0;
    while(1){
        for(p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++){
            tty_do_read(p_tty);
            tty_do_write(p_tty);
        }
    }
}

void in_process(TTY* p_tty, uint32_t key){
	char output[2] = {'\0', '\0'};
	if(!(key & FLAG_EXT)){ // is it's printable key
        if(p_tty->inbuf_count < TTY_IN_BYTES){
            *(p_tty->p_inbuf_head) = key;
            p_tty->p_inbuf_head++;
            if(p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES)
                p_tty->p_inbuf_head = p_tty->in_buf;
            p_tty->inbuf_count++;
        }
        /*
		output[0] = key & 0xff;
		kprint(output);

        int display_position = get_cursor();

        disable_int();
        out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_H);
        out_byte(CRTC_DATA_REG, ((display_position/2) >> 8) & 0xff);
        out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_L);
        out_byte(CRTC_DATA_REG, (display_position/2) & 0xff);

        enable_int();
        */
	}else{
        int row_code = key & MASK_RAW;
        switch(row_code){
            case UP:
                if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)){
                    // shift + up
                    disable_int();
                    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_H);
                    out_byte(CRTC_DATA_REG, ((80*15)>>8) & 0xff);
                    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_L);
                    out_byte(CRTC_DATA_REG, (80*15) & 0xff);
                    enable_int();
                }

                break;
            case DOWN:
                if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)){
                    // shift + down
                    
                }
                break;
            default:
                break;            
        }
    }
}