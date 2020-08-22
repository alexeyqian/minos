#include "ktest.h"
#include "clock.h"
#include "kio.h"
#include "fs.h"
#include "assert.h"
#include "string.h"
#include "proc.h"

PRIVATE void test_printf_a(){
	char tty_name[] = "/dev_tty0";
	int fd_stdin = open(tty_name, O_RDWR);
	//kassert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	//kassert(fd_stdout == 1);
	int i = 0;
	while(i < 2){
		printf(">>> printf i:%d, fd_out: %d\n", i++, fd_stdout);
		delay(5000);
	}
}

PUBLIC void test_a(){	
	while(1){}
	//test_printf_a();
	//kspin("test a");	
}

void test_b(){
	while(1){}
	kspin("test b");	
}

void test_c(){
	while(1){}
	while(1){	
		kprintf(">>> ticks: %d\n", 100);
		delay(5000);
	}
}