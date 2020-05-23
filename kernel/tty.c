#include "const.h"
#include "types.h"
#include "asm_util.h"
#include "klib.h"
#include "keyboard.h"
#include "tty.h"

// tty = keyboard (kb buffer) + console (screen buffer)
extern int     disp_pos;
extern TTY     tty_table[];
extern CONSOLE console_table[];
extern int nr_current_console;
#define TTY_FIRST (tty_table)
#define TTY_END   (tty_table + NR_CONSOLES)

void init_screen(TTY* p_tty);
void init_tty(TTY* p_tty){
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
    init_screen(p_tty);
}

bool_t is_current_console(CONSOLE* p_con){
    return (p_con == &console_table[nr_current_console]);
}

void set_video_start_addr(uint32_t addr){
    disable_int();
	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

static void set_cursor2(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

// keyboard interrupt handler put keys into keyboard buffer
// tty_do_read read key from keyboard buffer and put it to tty buffer
static void tty_do_read(TTY* p_tty){
    if(is_current_console(p_tty->p_console))
        keyboard_read(p_tty);
}

void flush(CONSOLE* p_con){
    set_cursor2(p_con->cursor);
    set_video_start_addr(p_con->current_start_addr);
}

void out_char(CONSOLE* p_con, char ch){    
    uint8_t* p_vmem = (uint8_t*)(V_MEM_BASE + p_con->cursor * 2);

    switch(ch){
        case '\n':
            if(p_con->cursor < p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH){
                p_con->cursor = p_con->original_addr + SCREEN_WIDTH * 
                ((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
            }
            break;
        case '\b':
            if(p_con->cursor > p_con->original_addr){
                p_con->cursor--;
                *(p_vmem - 2) = ' ';
                *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
            }
            break;
        default:
            if(p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1){
                *p_vmem++ = ch;
                *p_vmem++ = DEFAULT_CHAR_COLOR;
                p_con->cursor++;
            }
            break;
    }


    while(p_con->cursor >= p_con->current_start_addr +  SCREEN_SIZE)
        scroll_screen(p_con, SCROLL_SCREEN_DOWN);

    flush(p_con);
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

// TODO: rename to put_key_into_tty_buffer
void put_key(TTY* p_tty, uint32_t key){
    if(p_tty->inbuf_count < TTY_IN_BYTES){
        *(p_tty->p_inbuf_head) = key;
        p_tty->p_inbuf_head++;
        if(p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) // if full, then rewind
            p_tty->p_inbuf_head = p_tty->in_buf;

        p_tty->inbuf_count++;
    }
}

void in_process(TTY* p_tty, uint32_t key){
	char output[2] = {'\0', '\0'};
	if(!(key & FLAG_EXT)){ // is it's printable key
        put_key(p_tty, key);
	}else{
        int raw_code = key & MASK_RAW;
        switch(raw_code){
            case ENTER:
                put_key(p_tty, 0x0a);
                break;
            case BACKSPACE:
                put_key(p_tty, '\b');
                break;
            case UP:
                if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) // shift + up
                    scroll_screen(p_tty->p_console, SCROLL_SCREEN_UP);
                break;
            case DOWN:
                if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) //shift + down
                    scroll_screen(p_tty->p_console, SCROLL_SCREEN_DOWN);               
                break;
            default:
                break;            
        }
    }
}

void init_screen(TTY* p_tty){
    int nr_tty = p_tty - tty_table;
    p_tty->p_console = console_table + nr_tty;
    int v_mem_size   = V_MEM_SIZE >> 1;  // video memo size in word
    int con_v_mem_size = v_mem_size / NR_CONSOLES;
    p_tty->p_console->original_addr = nr_tty * con_v_mem_size;
    p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;
    p_tty->p_console->cursor = p_tty->p_console->original_addr;
    if(nr_tty == 0) // first console use current position
        p_tty->p_console->cursor = disp_pos / 2;
    else
    {
        out_char(p_tty->p_console, nr_tty + '0');
        out_char(p_tty->p_console, '#');
    }

    set_cursor2(p_tty->p_console->cursor);
}




// 0 ~ NR_CONSOLES - 1
void select_console(int nr_console) {
    if((nr_console < 0) || (nr_console >= NR_CONSOLES)) return; // invalid number

    nr_current_console = nr_console;
    set_cursor2(console_table[nr_console].cursor);
    set_video_start_addr(console_table[nr_console].current_start_addr);
}

void scroll_screen(CONSOLE* p_con, int direction){
    if(direction == SCROLL_SCREEN_UP){
        if(p_con->current_start_addr > p_con->original_addr)
            p_con->current_start_addr -= SCREEN_WIDTH;

    }else if(direction == SCROLL_SCREEN_DOWN){
        if(p_con->current_start_addr + SCREEN_SIZZE < p_con->original_addr + p_con->v_mem_limit)
            p_con->current_start_addr += SCREEN_WIDTH;
    }else{

    }
    set_video_start_addr(p_con->current_start_addr);
    set_cursor(p_con->cursor);
}