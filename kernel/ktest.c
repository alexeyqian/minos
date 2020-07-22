#include "ktest.h"
#include "ke_asm_utils.h"
#include "klib.h"
#include "clock.h"
#include "interrupt.h"
#include "ipc.h"
#include "screen.h"

void test_a(){	
	int i = 0;	
	while(1){	
		kprint("A"); 
		printf("X:");	
		//printf(">>> hardcode: %x", 100); // TODO: NOT WORKING ... PROBLEM CODE
		//printf("<Ticks: %d>", get_ticks());
		delay(5000);
	}
}

void test_b(){
	int i = 0x1000;
	while(1){	
		kprint("B");
		//printf("<Ticks2: %d>", get_ticks2());	
		delay(5000);
	}
}

void test_c(){
	int i = 0x2000;
	int counter = 10;
	while(1){	
		//printf("Z");
		delay(5000);
	}
}