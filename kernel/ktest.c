#include "ktest.h"
#include "clock.h"
#include "kio.h"

void test_a(){	
	int i = 100;	
	while(1){	
		printf("<A: %x %d>", get_ticks(), i++); 
		delay(5000);
	}
}

void test_b(){
	int i = 200;
	while(1){	
		printf("<B: %x %d>", get_ticks2(), i++); 
		delay(5000);
	}
}

void test_c(){
	int i = 0x2000;
	while(1){	
		//printf("<C: %d>", i++);
		delay(5000);
	}
}