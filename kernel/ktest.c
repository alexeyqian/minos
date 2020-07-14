#include "ktest.h"
#include "ke_asm_utils.h"
#include "klib.h"
#include "interrupt.h"


void test_a(){	
	int i = 0;	
	while(1){	
		kprint("A"); // user process, don't have permission of IO	
		//kprint("A \n");
		printf("X");	
		//printf("<Ticks: %x>", get_ticks());
		milli_delay(2000);
		//delay(10);		
	}
}

void test_b(){
	int i = 0x1000;
	while(1){	
		kprint("B");
		printf("Y");	
		milli_delay(2000);
	}
}

void test_c(){
	int i = 0x2000;
	int counter = 10;
	while(1){	
		kprint("C");
		printf("Z");
		milli_delay(2000);
	}
}