#include "const.h"
#include "types.h"
#include "asm_util.h"
#include "klib.h"
#include "keymap.h"
#include "keyboard.h"
#include "shared.h"
#include "keyboard.h"

// static is for compiler only, will generate same asm code with or without it.
// statix will cause the symble not be included in export symbol table,
// which in turn will reduce linkage time, since less symbols in the table to process.
static KB_INPUT kb_in;
static bool_t code_with_e0;
static bool_t shift_l; 
static bool_t shift_r;
static bool_t alt_l;
static bool_t alt_r;
static bool_t ctrl_l;
static bool_t ctrl_r;
static bool_t caps_lock;
static bool_t num_lock;
static bool_t scroll_lock;
static int 	  column = 0; // keyrow[column]: a value in keymap

// static for function means private to this module.
static void keyboard_handler(int irq);
void init_keyboard(){
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;

	caps_lock = 0;
	num_lock = 1;
	scroll_lock = 0;
	set_leds();

	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
}

// write to buffer
static void keyboard_write(){
	//kprint("code arrive keyboard_handler!");
	uint8_t scan_code = in_byte(KB_DATA);
	//print_int(scan_code);

	if(kb_in.count < KB_IN_BYTES){
		*(kb_in.p_head) = scan_code;
		kb_in.p_head++;

		if(kb_in.p_head == kb_in.buf + KB_IN_BYTES)
			kb_in.p_head = kb_in.buf;
		
		kb_in.count++;
	}
}

// for 1 byte  scan code, each keypress/release fires 2 interrupts (1 make code and 1 break code)
// for 2 bytes scan code, each keypress/release fires 4 interrupts (2 make codes and 2 break codes, start with E0)
// for 3 bytes scan code, PUASE, only has make code, no break code, so only fires 3 interrupts (start with E1)
// one for make code (press), one for break code (release).
// scan code has 2 types: mark code and break code
void keyboard_handler(int irq){
	keyboard_write();
}

uint8_t get_byte_from_kb_buf()	/* 从键盘缓冲区中读取下一个字节 */
{
	uint8_t	scan_code;

	while (kb_in.count <= 0) {}	/* 等待下一个字节到来 */

	disable_int();
	scan_code = *(kb_in.p_tail);
	kb_in.p_tail++;
	if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
		kb_in.p_tail = kb_in.buf;
	}
	kb_in.count--;
	enable_int();

	return scan_code;
}

extern void in_process(uint32_t key);

// read from buffer
// it might run multiple times to get a complete scan code.
// since some scan code contains more than one bytes
void keyboard_read()
{
	uint8_t	    scan_code;
	bool_t	    make;	   /* TRUE : make  FALSE: break */
	uint32_t	key = 0;   /* 用一个整型来表示一个键。 */
	uint32_t*	keyrow;	   /* 指向 keymap[] 的某一行 */

	if(kb_in.count > 0){
		code_with_e0 = FALSE;
		scan_code = get_byte_from_kb_buf();
		
		if (scan_code == 0xE1) {
			int i;
			uint8_t pausebreak_scan_code[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
			bool_t is_pausebreak = TRUE;
			for(i=1;i<6;i++){
				if (get_byte_from_kb_buf() != pausebreak_scan_code[i]) {
					is_pausebreak = FALSE;
					break;
				}
			}
			if (is_pausebreak) {
				key = PAUSEBREAK;
			}
		}
		else if (scan_code == 0xE0) {
			scan_code = get_byte_from_kb_buf();

			/* PrintScreen 被按下 */
			if (scan_code == 0x2A) {
				if (get_byte_from_kb_buf() == 0xE0) {
					if (get_byte_from_kb_buf() == 0x37) {
						key = PRINTSCREEN;
						make = TRUE;
					}
				}
			}

			/* PrintScreen 被释放 */
			if (scan_code == 0xB7) {
				if (get_byte_from_kb_buf() == 0xE0) {
					if (get_byte_from_kb_buf() == 0xAA) {
						key = PRINTSCREEN;
						make = FALSE;
					}
				}
			}

			/* 不是 PrintScreen。此时 scan_code 为 0xE0 紧跟的那个值。 */
			if (key == 0) {
				code_with_e0 = TRUE;
			}
		}
		
		if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
			/* 首先判断Make Code 还是 Break Code */
			make = (scan_code & FLAG_BREAK ? FALSE : TRUE);			
			/* 先定位到 keymap 中的行 */
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];

			column = 0;
			
			if (shift_l || shift_r) {
				//kprint("yes shift");
				column = 1;
			}else kprint("no shift");

			if (code_with_e0) {
				column = 2;
			}

			key = keyrow[column];			
			switch(key) {
				case SHIFT_L:
					shift_l	= make;
					break;
				case SHIFT_R:
					shift_r	= make;
					break;
				case CTRL_L:
					ctrl_l	= make;
					break;
				case CTRL_R:
					ctrl_r	= make;
					break;
				case ALT_L:
					alt_l	= make;
					break;
				case ALT_R:
					alt_l	= make;
					break;
				case CAPS_LOCK:
					if(make){
						caps_lock = !caps_lock;
						set_leds();
					}
					break;
				case NUM_LOCK:
					if(make){
						num_lock = !num_lock;
						set_leds();
					}
					break;
				case SCROLL_LOCK:
					if(make){
						scroll_lock = !scroll_lock;
						set_leds();
					}
					break;
				default:
					break;
			}
		}

		if(make){ // ignore Break Code	

			bool_t pad = FALSE;
			// process pad keyboard first
			if((key >= PAD_SLASH) && (key <= PAD_9)){
				pad = TRUE;
				switch(key) { // + - * / in number pad
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if (num_lock) {	/* '0' ~ '9' and '.' in num pad */
							if ((key >= PAD_0) && (key <= PAD_9)) {
								key = key - PAD_0 + '0';
							}
							else if (key == PAD_DOT) {
								key = '.';
							}
						}
						else{
							switch(key) {
								case PAD_HOME:
									key = HOME;
									break;
								case PAD_END:
									key = END;
									break;
								case PAD_PAGEUP:
									key = PAGEUP;
									break;
								case PAD_PAGEDOWN:
									key = PAGEDOWN;
									break;
								case PAD_INS:
									key = INSERT;
									break;
								case PAD_UP:
									key = UP;
									break;
								case PAD_DOWN:
									key = DOWN;
									break;
								case PAD_LEFT:
									key = LEFT;
									break;
								case PAD_RIGHT:
									key = RIGHT;
									break;
								case PAD_DOT:
									key = DELETE;
									break;
								default:
									break;
							}
						}
						break;	
				}
			}

			key |= shift_l	? FLAG_SHIFT_L	: 0;
			key |= shift_r	? FLAG_SHIFT_R	: 0;
			key |= ctrl_l	? FLAG_CTRL_L	: 0;
			key |= ctrl_r	? FLAG_CTRL_R	: 0;
			key |= alt_l	? FLAG_ALT_L	: 0;
			key |= alt_r	? FLAG_ALT_R	: 0;

			key |= pad      ? FLAG_PAD      : 0;
			in_process(key);
		}
	}
}

// wait for 8042 buffer is empty
static void kb_wait(){
	uint_8 kb_stat;
	do{
		kb_stat = in_byte(KB_CMD);
	}while(kb_stat & 0x2);
} 

static void kb_ack(){
	uint8_t kb_read;
	do{
		kb_read = in_byte(KB_DATA);
	}while(kb_read != KB_ACK);
}


static void set_leds(){
	uint8_t leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;
	kb_wait();
	out_byte(KB_DATA, LED_CODE);
	kb_ack();
	kb_wait();
	out_byte(KB_DATA, leds);
	kb_ack();
}