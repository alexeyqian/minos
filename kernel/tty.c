#include "const.h"
#include "types.h"
#include "ke_asm_utils.h"
#include "klib.h"
#include "keyboard.h"
#include "tty.h"

// tty = one shared keyboard (kb buffer) + multiple consoles (screen buffer)
extern TTY tty_table[];

#define TTY_FIRST (tty_table)
#define TTY_END (tty_table + NR_CONSOLES)

PRIVATE CONSOLE console_table[NR_CONSOLES];
PRIVATE int current_console_idx = 0;
PRIVATE int _disp_pos;

// ==== console ===========

void tty_output_char(CONSOLE *p_con, char ch);

PRIVATE void tty_set_cursor(unsigned int position)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}

PRIVATE void init_console(CONSOLE *p_console, int idx)
{    
    int v_mem_size_in_word = V_MEM_SIZE >> 1;
    int v_mem_size_in_word_per_con = v_mem_size_in_word / NR_CONSOLES;
    p_console->original_addr = idx * v_mem_size_in_word_per_con;
    p_console->v_mem_limit = v_mem_size_in_word_per_con; // /NR_CONSOLES * NR_CONSOLES ??
    p_console->current_start_addr = p_console->original_addr;
    p_console->cursor = p_console->original_addr;
    if (idx == 0)
    {   // first console use current position
        p_console->cursor = _disp_pos / 2;
        _disp_pos = 0; 
    }
    else
    {
        tty_output_char(p_console, idx + '0');
        tty_output_char(p_console, '#');
    }
    
    tty_set_cursor(p_console->cursor);
}

PRIVATE void tty_set_video_start_addr(uint32_t addr)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr & 0xFF);
    enable_int();
}

PRIVATE void scroll_screen(CONSOLE *p_con, int direction)
{
    if (direction == SCROLL_SCREEN_UP)
    {
        if (p_con->current_start_addr > p_con->original_addr){
            p_con->current_start_addr -= SCREEN_WIDTH;
        }            
    }
    else if (direction == SCROLL_SCREEN_DOWN)
    {
        if (p_con->current_start_addr + SCREEN_SIZE < p_con->original_addr + p_con->v_mem_limit){
            p_con->current_start_addr += SCREEN_WIDTH;
        }            
    }
    else{}

    tty_set_video_start_addr(p_con->current_start_addr);
    tty_set_cursor(p_con->cursor);
}

void tty_output_char(CONSOLE *p_con, char ch)
{
    uint8_t *p_vmem = (uint8_t *)(V_MEM_BASE + p_con->cursor * 2);

    switch (ch)
    {
        case '\n':
            if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH)
            {
                p_con->cursor = p_con->original_addr + SCREEN_WIDTH *
                                                        ((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
            }
            break;
        case '\b':
            if (p_con->cursor > p_con->original_addr)
            {
                p_con->cursor--;
                *(p_vmem - 2) = ' ';
                *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
            }
            break;
        default:
            if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1)
            {
                *p_vmem++ = ch;
                *p_vmem++ = DEFAULT_CHAR_COLOR;
                p_con->cursor++;
            }
            break;
    }

    if (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE)
       scroll_screen(p_con, SCROLL_SCREEN_DOWN);

    tty_set_cursor(p_con->cursor);
}

PRIVATE void select_console(int con_idx)
{
    if ((con_idx < 0) || (con_idx >= NR_CONSOLES))
        return; // invalid number

    current_console_idx = con_idx;
    tty_set_cursor(console_table[con_idx].cursor);
    tty_set_video_start_addr(console_table[con_idx].current_start_addr);
}

PRIVATE bool_t is_current_console(CONSOLE *p_con)
{
    return (p_con == &console_table[current_console_idx]);
}

// keyboard interrupt handler put keys into keyboard buffer
// tty_do_read read key from keyboard buffer and put it to tty buffer
PRIVATE void tty_do_read(TTY *p_tty)
{
    if (is_current_console(p_tty->p_console))
        kb_read(p_tty);
}

// prerequisite: tty buf must not empty
PRIVATE char retrive_char_from_tty_buf(TTY *p_tty)
{
    //disable_int();
    char ch = *(p_tty->p_inbuf_tail);
    p_tty->p_inbuf_tail++;
    if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES)
        p_tty->p_inbuf_tail = p_tty->in_buf;
    p_tty->inbuf_count--;
    //enable_int();
    return ch;
}

PRIVATE void tty_do_write(TTY *p_tty)
{
    if (p_tty->inbuf_count)
    { // tty_buff is not empty
        char ch = retrive_char_from_tty_buf(p_tty);
        tty_output_char(p_tty->p_console, ch);
    }
}

// ===== tty = keyboard + console(screen) =====
PRIVATE void init_tty(TTY *p_tty)
{
    // inti tty buffer
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

    int idx = p_tty - tty_table;    
    p_tty->p_console = console_table + idx;
    init_console(p_tty->p_console, idx);
}

void task_tty()
{       
    TTY *p_tty;

    init_keyboard();    
    for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
        init_tty(p_tty);

    select_console(0);
    
    while (1)
    {
        for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
        {
            tty_do_read(p_tty);
            tty_do_write(p_tty);
        }
    }
}

PRIVATE void append_combined_key_to_tty_buf(TTY *p_tty, uint32_t key)
{   
    if (p_tty->inbuf_count < TTY_IN_BYTES)
    {
        *(p_tty->p_inbuf_head) = key;
        p_tty->p_inbuf_head++;
        if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) // if full, then rewind
            p_tty->p_inbuf_head = p_tty->in_buf;

        p_tty->inbuf_count++;
    }
}

PRIVATE void process_printable_key(TTY *p_tty, uint32_t combined_key)
{
    append_combined_key_to_tty_buf(p_tty, combined_key);
}

PRIVATE void process_command_key(TTY *p_tty, uint32_t combined_key)
{
    int raw_code = combined_key & MASK_RAW; // 9 bits raw command code
    switch (raw_code)
    {
    case ENTER:
        append_combined_key_to_tty_buf(p_tty, '\n');
        break;
    case BACKSPACE:
        append_combined_key_to_tty_buf(p_tty, '\b');
        break;
    case UP:
        if ((combined_key & FLAG_SHIFT_L) || (combined_key & FLAG_SHIFT_R)) // shift + up
            scroll_screen(p_tty->p_console, SCROLL_SCREEN_UP);
        break;
    case DOWN:
        if ((combined_key & FLAG_SHIFT_L) || (combined_key & FLAG_SHIFT_R)) //shift + down
            scroll_screen(p_tty->p_console, SCROLL_SCREEN_DOWN);
        break;
    case F1:
    case F2:
    case F3:
    case F4:
    case F5:
    case F6:
    case F7:
    case F8:
    case F9:
    case F10:
    case F11:
    case F12:
        if ((combined_key & FLAG_ALT_L) || (combined_key & FLAG_ALT_R))
        { /* Alt + F1~F12 */
            select_console(raw_code - F1);
        }
        break;
    default:
        break;
    }
}

void hand_over_key_to_tty(TTY *p_tty, uint32_t combined_key)
{
    if (!(combined_key & FLAG_EXT)) // 8 bits printable code
        process_printable_key(p_tty, combined_key);
    else // 9 bits raw command code
        process_command_key(p_tty, combined_key);
}