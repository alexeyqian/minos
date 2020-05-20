#include "ktest.h"
#include "klib.h"

void test_a(){
	int i = 0;
	while(1){
		//get_ticks();
		kprint("A");
		print_int(i++);
		kprint(".");
		delay(1);
	}
}

void test_b(){
	int i = 0x1000;
	while(1){
		kprint("B");
		print_int(i++);
		kprint(".");
		delay(1);
	}
}

void test_c(){
	int i = 0x2000;
	while(1){
		kprint("C");
		print_int(i++);
		kprint(".");
		delay(1);
	}
}