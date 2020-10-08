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

#include "keymap.h"

// static is for compiler only, will generate same asm code with or without it.
// static will cause the symble not be included in export symbol table,
// which in turn will reduce linkage time, since less symbols in the table to process.

PRIVATE bool_t code_with_E0 = FALSE;
PRIVATE bool_t shift_l; 
PRIVATE bool_t shift_r;
PRIVATE bool_t alt_l;
PRIVATE bool_t alt_r;
PRIVATE bool_t ctrl_l;
PRIVATE bool_t ctrl_r;
PRIVATE bool_t caps_lock = 0;   
PRIVATE bool_t num_lock = 0;    
PRIVATE bool_t scroll_lock = 0; 
PRIVATE bool_t column = 0; // keyrow[column]: a value in keymap

// wait for an empty keyboard controller(8042) buffer to write
PRIVATE void kb_wait(){
	uint8_t kb_stat;
	do{
		kb_stat = inb(KB_CMD);
	}while(kb_stat & 0x02);
} 

PRIVATE void kb_ack(){
	uint8_t kb_byte;
	do{
		kb_byte = inb(KB_DATA);
	}while(kb_byte != KB_ACK);
}

PRIVATE void set_leds(){
	int leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;
	kb_wait();
	outb(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	outb(KB_DATA, (uint8_t)leds); // safe here
	kb_ack();
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
PUBLIC uint32_t kb_read() 
{
	uint8_t	    scan_code;
	bool_t	    is_make_code;	               // true for make code, false for break code
	uint32_t	key = 0;        
	uint32_t*	keyrow;	               // points to row of keymap[]

	//if(kb_in.count <= 0) return; // keyboard buffer is empty // TODO
	
	code_with_E0 = FALSE;
	scan_code = retrive_scan_code_from_kb_buf(); // kcall warpper

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

	if(!is_make_code) return 0;  // ignore break code	// TODO: return ?? - check if 0 in caller?
	
	// in our system ascii printable keys are defined as 8 bits value
	// in our system ascii special   keys are defined as 9 bits value
	// using 32 bits to combine make code and status, such as shift/ctrl/alt
	uint32_t combinded_key = process_make_code(key);
    return combinded_key;
}

PUBLIC void init_keyboard(){	
	shift_l = shift_r = 0;
	alt_l   = alt_r   = 0;
	ctrl_l  = ctrl_r  = 0;

	caps_lock = 0;
	num_lock = 1;
	scroll_lock = 0;
	set_leds();

	enableirq(KEYBOARD_IRQ);
}
