#include "const.h"
#include "types.h"
#include "ke_asm_utils.h"
#include "klib.h"
#include "keymap.h"
#include "keyboard.h"
#include "shared.h"
#include "keyboard.h"
#include "tty.h"

// static is for compiler only, will generate same asm code with or without it.
// static will cause the symble not be included in export symbol table,
// which in turn will reduce linkage time, since less symbols in the table to process.
PRIVATE KB_INPUT kb_in;
PRIVATE bool_t code_with_E0 = FALSE;
PRIVATE bool_t shift_l; 
PRIVATE bool_t shift_r;
PRIVATE bool_t alt_l;
PRIVATE bool_t alt_r;
PRIVATE bool_t ctrl_l;
PRIVATE bool_t ctrl_r;
PRIVATE int caps_lock;
PRIVATE int num_lock;
PRIVATE int scroll_lock;
PRIVATE int    column = 0; // keyrow[column]: a value in keymap

// wait for an empty keyboard controller(8042) buffer to write
PRIVATE void kb_wait(){
	uint8_t kb_stat;
	do{
		kb_stat = in_byte(KB_CMD);
	}while(kb_stat & 0x02);
} 

PRIVATE void kb_ack(){
	uint8_t kb_read;
	do{
		kb_read = in_byte(KB_DATA);
	}while(kb_read != KB_ACK);
}

PRIVATE void set_leds(){
	uint8_t leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;
	kb_wait();
	out_byte(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	out_byte(KB_DATA, leds);
	kb_ack();
}

PRIVATE uint8_t retrive_scan_code_from_kb_buf()	
{
	uint8_t	scan_code;
	while (kb_in.count <= 0) {} // waiting for at least one scan code

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

PRIVATE void append_scan_code_to_kb_buf(){	
	uint8_t scan_code = in_byte(KB_DATA);   // read out from 8042, so it can response for next key interrupt.
	if(kb_in.count >= KB_IN_BYTES) return;  // ignre scan code if buffer is full
	
	*(kb_in.p_head) = scan_code;
	kb_in.p_head++;

	if(kb_in.p_head == kb_in.buf + KB_IN_BYTES)
		kb_in.p_head = kb_in.buf;
	
	kb_in.count++;	
}

/*
	// for 1 byte  scan code, each keypress/release fires 2 interrupts (1 make code and 1 break code)
	// for 2 bytes scan code, each keypress/release fires 4 interrupts (2 make codes and 2 break codes, start with E0)
	// for 3 bytes scan code, PUASE, only has make code, no break code, so only fires 3 interrupts (start with E1)
	// one for make code (press), one for break code (release).
	// scan code has 2 types: make code and break code
*/
// Called per interrupt
PRIVATE void keyboard_handler(int irq){
	append_scan_code_to_kb_buf();
}

// key must be make key
PRIVATE uint32_t process_make_code(uint32_t key){
		bool_t pad = FALSE;

		// process pad keyboard
		if ((key >= PAD_SLASH) && (key <= PAD_9)) {
			pad = TRUE;
			switch(key) {	/* '/', '*', '-', '+', and 'Enter' in num pad  */
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
			default:	/* keys whose value depends on the NumLock */
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
		key |= pad	    ? FLAG_PAD	: 0;

		return key;
}

// read from buffer
// it might run multiple times to get a complete scan code.
// since some scan code contains more than one bytes
// make  code | 0x80 = break code
// break code & 0x7f = make  code
PUBLIC void kb_read(TTY* p_tty) 
{
	uint8_t	    scan_code;
	bool_t	    is_make_code;	               // true for make code, false for break code
	uint32_t	key = 0;        
	uint32_t*	keyrow;	               // points to row of keymap[]

	if(kb_in.count <= 0) return; // keyboard buffer is empty
	
	code_with_E0 = FALSE;
	scan_code = retrive_scan_code_from_kb_buf();

	if (scan_code == 0xE1) {
		int i;
		uint8_t pausebreak_scan_code[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
		bool_t is_pausebreak = TRUE;
		for(i=1;i<6;i++){
			if (retrive_scan_code_from_kb_buf() != pausebreak_scan_code[i]) {
				is_pausebreak = FALSE;
				break;
			}
		}
		if (is_pausebreak) {
			key = PAUSEBREAK;
		}
	}
	else if (scan_code == 0xE0) {
		code_with_E0 = TRUE;
		scan_code = retrive_scan_code_from_kb_buf();

		// PrintScreen is pressed
		if (scan_code == 0x2A) {
			code_with_E0 = FALSE;
			if ((scan_code = retrive_scan_code_from_kb_buf()) == 0xE0) {
				code_with_E0 = TRUE;
				if ((scan_code = retrive_scan_code_from_kb_buf()) == 0x37) {
					key = PRINTSCREEN;
					is_make_code = TRUE;
				}
			}
		}
		// PrintScreen is released
		else if (scan_code == 0xB7) {
			code_with_E0 = FALSE;
			if ((scan_code = retrive_scan_code_from_kb_buf()) == 0xE0) {
				code_with_E0 = TRUE;
				if ((scan_code = retrive_scan_code_from_kb_buf()) == 0xAA) {
					key = PRINTSCREEN;
					is_make_code = FALSE;
				}
			}
		}
	}
	// 如果不是 PrintScreen。则此时 scan_code 为 0xE0 紧跟的那个值。 
	if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
		
		is_make_code = (scan_code & FLAG_BREAK ? FALSE : TRUE);
		keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
		column = 0;

		bool_t caps = shift_l || shift_r;
		if (caps_lock) {
			if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z')){
				caps = !caps;
			}
		}
		if (caps) {
			column = 1;
		}

		if (code_with_E0) {
			column = 2;
		}

		key = keyrow[column];

		switch(key) {
		case SHIFT_L:
			shift_l	= is_make_code;
			break;
		case SHIFT_R:
			shift_r	= is_make_code;
			break;
		case CTRL_L:
			ctrl_l	= is_make_code;
			break;
		case CTRL_R:
			ctrl_r	= is_make_code;
			break;
		case ALT_L:
			alt_l	= is_make_code;
			break;
		case ALT_R:
			alt_l	= is_make_code;
			break;
		case CAPS_LOCK:
			if (is_make_code) {
				caps_lock   = !caps_lock;
				set_leds();
			}
			break;
		case NUM_LOCK:
			if (is_make_code) {
				num_lock    = !num_lock;
				set_leds();
			}
			break;
		case SCROLL_LOCK:
			if (is_make_code) {
				scroll_lock = !scroll_lock;
				set_leds();
			}
			break;
		default:
			break;
		}
	}

	if(!is_make_code) return;  // ignore break code	
	
	// in our system ascii printable keys are defined as 8 bits value
	// in our system ascii special   keys are defined as 9 bits value
	// using 32 bits to combine make code and status, such as shift/ctrl/alt
	uint32_t combinded_key = process_make_code(key);
	hand_over_key_to_tty(p_tty, combinded_key);
}

// TODO: move out of tty, add it in kmain?
PUBLIC void init_keyboard(){
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;

	shift_l = shift_r = 0;
	alt_l   = alt_r   = 0;
	ctrl_l  = ctrl_r  = 0;

	caps_lock = 0;
	num_lock = 1;
	scroll_lock = 0;
	set_leds();

	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
}
