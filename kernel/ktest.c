#include "ktest.h"
#include "ke_asm_utils.h"
#include "klib.h"
#include "interrupt.h"

void test_a(){
	int i = 0;
	while(1){	
		//kprint("A");
		//print_int(i++);
		//kprint("}");
		//printf("<Ticks: %x>", get_ticks());
		milli_delay(200);
	}
}

void test_b(){
	int i = 0x1000;
	while(1){		
		//kprint("B");
		//print_int(i++);
		//kprint("}");		
		//printf("B");
		milli_delay(200);
	}
}

void test_c(){
	int i = 0x2000;
	while(1){
		//kprint("");
		//print_int(i++);
		//kprint("}");
		//printf("C");
		milli_delay(200);
	}
}