#include "ktest.h"
#include "clock.h"
#include "kio.h"
#include "fs.h"
#include "assert.h"
#include "string.h"

void test_a(){	
	int fd, n;
	const char filename[] = "blah";
	const char bufw[] = "cbaed";
	const int rd_bytes = 3;
	char bufr[rd_bytes];

	assert(rd_bytes <= strlen(bufw));

	// create
	fd = open(filename, O_CREAT | O_RDWR);
	assert(fd != -1);
	printl("File created. fd: %d\n", fd);
	
	size_t temp = strlen(bufw);
	printl("len: %d\n", temp);

	// write
	n = write(fd, bufw, strlen(bufw));
	printl("n: %d\n", n);	
	assert(n == strlen(bufw));
	printl("before close fd %d\n", fd);
	close(fd);
	printl("after close fd: %d\n", fd);
	// open
	fd = open(filename, O_RDWR);
	assert(fd != -1);
	printl("file opened. fd: %d\n", fd);

	// read
	n = read(fd, bufr, rd_bytes);
	assert(n == rd_bytes);
	bufr[n] = 0;
	printl("%d bytes read: %s\n", n, bufr);

	close(fd);
	spin("test a");
	/*
	int fd = open("/blah", O_CREAT);
	printf("fd: %d\n", fd);
	close(fd);
	spin("test a");
	//int i = 100;	
	while(1){	
		//printf("<A: %x %d>", get_ticks(), i++); 
		
		delay(5000);
	}*/
}

void test_b(){
	
	//int i = 200;
	while(1){	
		//printf("<B: %x %d>", get_ticks2(), i++); 
		delay(5000);
	}
}

void test_c(){
	//int i = 0x2000;
	while(1){	
		//printf("<C: %d>", i++);
		delay(5000);
	}
}