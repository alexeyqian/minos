#include "_tty.h"

PRIVATE TTY tty_table[NR_CONSOLES];
#define TTY_FIRST (tty_table)
#define TTY_END (tty_table + NR_CONSOLES)

PRIVATE CONSOLE console_table[NR_CONSOLES];
PRIVATE int current_console_idx = 0;

PUBLIC void tty_output_char(CONSOLE *p_con, char ch);

PRIVATE unsigned int tty_get_cursor(){
    return getcursor()/2;
}

PRIVATE void tty_set_cursor(unsigned int position) 
{
    setcursor(position);
}

PRIVATE void tty_set_video_start_addr(uint32_t addr)
{
    setvideostartaddr(addr);
}

/*
PRIVATE void tty_reset_start_addr(){
    outb(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_H);
	outb(CRTC_DATA_REG, 0);
	outb(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_L);
	outb(CRTC_DATA_REG, 0);
}*/

PRIVATE bool_t is_current_console(CONSOLE *con)
{
    return (con == &console_table[current_console_idx]);
}

PRIVATE void flush(CONSOLE* p_con)
{
	if (is_current_console(p_con)) {
		tty_set_cursor(p_con->cursor);
		tty_set_video_start_addr(p_con->current_start_addr);
	}
}

PRIVATE void init_console(CONSOLE *p_console, int idx)
{    
    // vars related to position and size below are in WORDS, not in BYTES.
    int v_mem_size_in_word = V_MEM_SIZE >> 1;
    int v_mem_size_in_word_per_con = v_mem_size_in_word / NR_CONSOLES;
    p_console->original_addr = idx * v_mem_size_in_word_per_con;
    p_console->size_in_word = v_mem_size_in_word_per_con / SCREEN_WIDTH * SCREEN_WIDTH;
    p_console->current_start_addr = p_console->original_addr;
    p_console->cursor = p_console->original_addr;
    if (idx == 0)
    {   // first console use current position
        p_console->cursor = tty_get_cursor();
    }
    else
    {
        //`?' in this string will be replaced with 0, 1, 2, ...		
		const char prompt[] = "[TTY #?]\n";
		const char* p = prompt;
		for (; *p; p++)
			tty_output_char(p_console, *p == '?' ? idx + '0' : *p);
    }
    
    tty_set_cursor(p_console->cursor);
}

PRIVATE void scroll_screen(CONSOLE* con, int dir)
{   /*
	 * variables below are all in-console-offsets (based on con->orig)
	 */
	int oldest; /* addr of the oldest available line in the console */
	int newest; /* .... .. ... latest ......... .... .. ... ....... */
	int scr_top;/* position of the top of current screen */

	newest = (con->cursor - con->original_addr) / SCREEN_WIDTH * SCREEN_WIDTH;
	oldest = con->is_full ? (newest + SCREEN_WIDTH) % con->size_in_word : 0;
	scr_top = con->current_start_addr - con->original_addr;

	if (dir == SCROLL_SCREEN_DOWN) {
		if (!con->is_full && scr_top > 0) {
			con->current_start_addr -= SCREEN_WIDTH;
		}
		else if (con->is_full && scr_top != oldest) {
			if (con->cursor - con->original_addr >= con->size_in_word - SCREEN_SIZE) {
				if (con->current_start_addr != con->original_addr)
					con->current_start_addr -= SCREEN_WIDTH;
			}
			else if (con->current_start_addr == con->original_addr) {
				scr_top = con->size_in_word - SCREEN_SIZE;
				con->current_start_addr = con->original_addr + scr_top;
			}
			else {
				con->current_start_addr -= SCREEN_WIDTH;
			}
		}
	}
	else if (dir == SCROLL_SCREEN_UP) {
		if (!con->is_full && newest >= scr_top + SCREEN_SIZE) {
			con->current_start_addr += SCREEN_WIDTH;
		}
		else if (con->is_full && scr_top + SCREEN_SIZE - SCREEN_WIDTH != newest) {
			if (scr_top + SCREEN_SIZE == (int)con->size_in_word)
				con->current_start_addr = con->original_addr;
			else
				con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else {
		assertx(dir == SCROLL_SCREEN_DOWN || dir == SCROLL_SCREEN_UP);
	}

	flush(con);
}

PUBLIC void clear_screen(int pos, int len)
{
	uint8_t* pch = (uint8_t*)(V_MEM_BASE + pos * 2);
	while (--len >= 0) {
		*pch++ = ' ';
		*pch++ = DEFAULT_CHAR_COLOR;
	}
}

/**
 * Copy data in WORDS.
 *
 * Note that the addresses of dst and src are not pointers, but integers, 'coz
 * in most cases we pass integers into it as parameters.
 * 
 * @param dst   Addr of destination.
 * @param src   Addr of source.
 * @param size  How many words will be copied.
 *****************************************************************************/
PRIVATE	void w_copy(unsigned int dst, const unsigned int src, size_t size)
{
	phys_copy((void*)(V_MEM_BASE + (dst << 1)),
		  (void*)(V_MEM_BASE + (src << 1)),
		  size << 1);
}

/* NOT USED
PRIVATE void tty_output_str(TTY* p_tty, char* buf, int len){
	char* p = buf;
	int i = len;
	while(i){
		tty_output_char(p_tty->p_console, *p++);
		i--;
	}
}*/

PUBLIC void tty_output_char(CONSOLE *con, char ch)
{
    uint8_t *p_vmem = (uint8_t *)(V_MEM_BASE + con->cursor * 2);
    assertx(con->cursor - con->original_addr < con->size_in_word);

    int cursor_x = (con->cursor - con->original_addr) % SCREEN_WIDTH;
    int cursor_y = (con->cursor - con->original_addr) / SCREEN_WIDTH;

    switch (ch)
    {
        case '\n':
            con->cursor = con->original_addr + SCREEN_WIDTH * (cursor_y + 1);
            break;
        case '\b':
            if (con->cursor > con->original_addr)
            {
                con->cursor--;
                *(p_vmem - 2) = ' ';
                *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
            }
            break;
        default:            
            *p_vmem++ = ch;
            *p_vmem++ = DEFAULT_CHAR_COLOR;
            con->cursor++;            
            break;
    }

    // tricks for infinete screen scroll
    if(con->cursor - con->original_addr >= con->size_in_word){
        cursor_x = (con->cursor - con->original_addr) % SCREEN_WIDTH;
		cursor_y = (con->cursor - con->original_addr) / SCREEN_WIDTH;
		int cp_orig = con->original_addr + (cursor_y + 1) * SCREEN_WIDTH - SCREEN_SIZE;
		w_copy(con->original_addr, cp_orig, SCREEN_SIZE - SCREEN_WIDTH);
		con->current_start_addr = con->original_addr;
		con->cursor = con->original_addr + (SCREEN_SIZE - SCREEN_WIDTH) + cursor_x;
		clear_screen((int)con->cursor, SCREEN_WIDTH);
		if (!con->is_full)
			con->is_full = 1;
    }

    assertx(con->cursor - con->original_addr < con->size_in_word);

    if (con->cursor >= con->current_start_addr + SCREEN_SIZE
        || con->cursor < con->current_start_addr){ // TODO: if <-> while?
       scroll_screen(con, SCROLL_SCREEN_UP);
       clear_screen((int)con->cursor, SCREEN_WIDTH);
    }

    flush(con);
}

PRIVATE void select_console(int con_idx)
{
    if ((con_idx < 0) || (con_idx >= NR_CONSOLES)) return; // invalid number

    current_console_idx = con_idx;
    flush(&console_table[current_console_idx]);
}

// prerequisite: tty buf must not empty
PRIVATE char retrive_char_from_tty_buf(TTY *p_tty)
{
    //clearintr(); //NO NEED
    uint32_t ch = *(p_tty->p_inbuf_tail);
    p_tty->p_inbuf_tail++;
    if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES)
        p_tty->p_inbuf_tail = p_tty->in_buf;
    p_tty->inbuf_count--;
    //setintr();
    return (char)ch;
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

/**
 * Get chars from the keyboard buffer if the tty is the current console.
 * keyboard interrupt handler put keys into keyboard buffer
 * tty_do_read read key from keyboard buffer and put it to tty buffer
 * */
PRIVATE void tty_dev_read(TTY *p_tty)
{
    if (!is_current_console(p_tty->p_console)) return;
    
    uint32_t combined_key = kb_read(p_tty);
    //printx("[dev read: %x] ", combined_key);
     if (!(combined_key & FLAG_EXT)) // 8 bits printable code
        process_printable_key(p_tty, combined_key);
    else // 9 bits raw command code
        process_command_key(p_tty, combined_key);    
}

/**
 * Echo the char just pressed and transfer it to the waiting process
 * */
PRIVATE void tty_dev_write(TTY *tty)
{
    while(tty->inbuf_count){
        char ch = retrive_char_from_tty_buf(tty);
        //printx("tty_left_cnt: %d.", tty->tty_left_cnt);
        //printx(" ch: %c ", ch);
        // tty_left_cnt is 0 before request from file system.
        if(tty->tty_left_cnt){
            if(ch >= ' ' && ch <= '~'){ // printable
                tty_output_char(tty->p_console, ch);
                void* p = tty->tty_req_buf + tty->tty_trans_cnt;
                phys_copy(p, (void*)va2la(TASK_TTY, &ch), 1);
                tty->tty_trans_cnt++;
                tty->tty_left_cnt--;
            }else if(ch == '\b' && tty->tty_trans_cnt){
                tty_output_char(tty->p_console, ch);
                tty->tty_trans_cnt--;
                tty->tty_left_cnt++;
            }

            if(ch == '\n' || tty->tty_left_cnt == 0){
                tty_output_char(tty->p_console, '\n');
                KMESSAGE msg;
                msg.type = RESUME_PROC;
                msg.PROC_NR = tty->tty_procnr;
                msg.CNT = tty->tty_trans_cnt;
                send_recv(SEND, tty->tty_caller, &msg);
                tty->tty_left_cnt = 0;
            }
        }
    }
}

PRIVATE void tty_do_read(TTY* tty, KMESSAGE* pmsg){
    tty->tty_caller = pmsg->source; // who called, usally task_fs
    tty->tty_procnr = pmsg->PROC_NR; // who wants the chars
    tty->tty_req_buf = va2la(tty->tty_procnr, pmsg->BUF); // where the chars should be put
    tty->tty_left_cnt = pmsg->CNT; // how many chars are requested
    tty->tty_trans_cnt = 0; // how many chars have been transferred

    pmsg->type = SUSPEND_PROC;
    pmsg->CNT = tty->tty_left_cnt;
    send_recv(SEND, tty->tty_caller, pmsg);
}

PRIVATE void tty_do_write(TTY* tty, KMESSAGE* pmsg){
    char buf[TTY_OUT_BUF_LEN];
    char* p = (char*)va2la(pmsg->PROC_NR, pmsg->BUF);
    int i = pmsg->CNT;
    int j;

    while(i){
        int bytes = min(TTY_OUT_BUF_LEN, i);
        assertx(bytes >= 0);
        phys_copy(va2la(TASK_TTY, buf), (void*)p, (size_t)bytes);
        for(j = 0; j < bytes; j++)
            tty_output_char(tty->p_console, buf[j]);
        
        i -= bytes;
        p += bytes;
    }

    pmsg->type = SYSCALL_RET;
    send_recv(SEND, pmsg->source, pmsg);
}

// ===== tty = keyboard + console(screen) =====
PRIVATE void init_tty(TTY *p_tty)
{
    // inti tty buffer
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
    p_tty->tty_left_cnt = 0; // IMPORTANT

    int idx = p_tty - tty_table;    
    p_tty->p_console = console_table + idx;
    init_console(p_tty->p_console, idx);
}

PUBLIC void svc_tty()
{  
    printx(">>> 4. service tty is running.\n"); 
    TTY* p_tty;
    KMESSAGE msg;
    
    init_keyboard();

    for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
        init_tty(p_tty);

    select_console(0);
    
    while (TRUE)
    {        
        for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
        {
            do{
                tty_dev_read(p_tty);
                tty_dev_write(p_tty);
            }while(p_tty->inbuf_count);            
        }

        send_recv(RECEIVE, ANY, &msg);

        int src = msg.source;
        assertx(src != TASK_TTY);

        TTY* ptty2 = &tty_table[msg.DEVICE];
        //printx(">>> tty main, type: %d, src: %d\n", msg.type, src);
        switch(msg.type){
            case DEV_OPEN: // nothing need to open, just return
                reset_msg(&msg);
                msg.type = SYSCALL_RET;
                //printx(">>> task_tty()::DEVOPEN begin, src: %d, type: %d\n", src, msg.type);
                send_recv(SEND, src, &msg);
                //printx(">>> task_tty()::DEVOPEN end, src: %d, type: %d\n", src, msg.type);
                break;
            case DEV_READ:
                tty_do_read(ptty2, &msg);
                break;
            case DEV_WRITE:
                tty_do_write(ptty2, &msg);
                break;
            case HARD_INT: // waked up by clock_handler
                setkeypressed(0);
                continue;
            default:
                //dump_msg("tty::unknown message", &msg);
                panicx("tty:unknow message.");
                break;
        }
    }
}
