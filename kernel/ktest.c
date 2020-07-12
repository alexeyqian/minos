#include "ktest.h"
#include "ke_asm_utils.h"
#include "klib.h"
#include "interrupt.h"

void test_a(){	
	int i = 0;
	while(1){	
		//kprint("A"); // TODO: ?? user process, don't have permission of IO, cannot call kprint any more	
		printf("A");	
		printf("<Ticks: %x>", get_ticks());
		milli_delay(2000);
	}
}

void test_b(){
	int i = 0x1000;
	while(1){		
		printf("B");	
		milli_delay(2000);
	}
}

void test_c(){
	int i = 0x2000;
	while(1){
		printf("C");
		milli_delay(2000);
	}
}