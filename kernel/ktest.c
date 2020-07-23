#include "ktest.h"
#include "ke_asm_utils.h"
#include "klib.h"
#include "clock.h"
#include "interrupt.h"
#include "ipc.h"
#include "screen.h"

void test_a(){	
	int i = 100;	
	while(1){	
		printf("<A: %d %d %d>", 100, 300, i++); 
		//kprint_int_as_hex(i++);
		//kprint_int_as_hex(get_ticks());
		//kprint(":");
		//kprint_int_as_hex(get_ticks2());
		delay(5000);
	}
}

void test_b(){
	int i = 200;
	while(1){	
		delay(5000);
	}
}

void test_c(){
	int i = 0x2000;
	while(1){	
		delay(5000);
	}
}