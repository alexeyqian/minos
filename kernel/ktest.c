#include "ktest.h"
#include "asm_util.h"
#include "klib.h"
#include "interrupt.h"

void test_a(){
	int i = 0;
	while(1){		
		//print_int(get_ticks());
		kprint("A");
		//print_int(i++);
		//kprint("}");
		milli_delay(200);
	}
}

void test_b(){
	int i = 0x1000;
	while(1){		
		kprint("B");
		//print_int(i++);
		//kprint("}");		
		milli_delay(200);
	}
}

void test_c(){
	int i = 0x2000;
	while(1){
		kprint("");
		//print_int(i++);
		//kprint("}");
		milli_delay(200);
	}
}