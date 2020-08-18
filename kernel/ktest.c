#include "ktest.h"
#include "clock.h"
#include "kio.h"
#include "fs.h"
#include "assert.h"
#include "string.h"
#include "proc.h"

void test_a(){	
	kspin("test a");	
}

void test_b(){
	kspin("test b");	
}

void test_c(){
	kspin("test c");	
	while(1){	
		//kprintf(">>> ticks: %d\n", get_ticks());
		//delay(5000); // TODO: delay issue here
	}
}