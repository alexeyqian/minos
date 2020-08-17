#include "ktest.h"
#include "clock.h"
#include "kio.h"
#include "fs.h"
#include "assert.h"
#include "string.h"
#include "proc.h"

void test_a(){	
	kspin("test_a");
	int fd, n, i;
	const char filename[] = "blah";
	const char bufw[] = "abcde";
	const int rd_bytes = 3;
	char bufr[rd_bytes];

	assert(rd_bytes <= strlen(bufw));

	// create
	fd = open(filename, O_CREAT | O_RDWR);
	assert(fd != -1);
	kprintf("File created. fd: %d\n", fd);
	
	// write
	n = write(fd, bufw, strlen(bufw));
	assert(n == strlen(bufw));

	close(fd);
	
	// open
	fd = open(filename, O_RDWR);
	assert(fd != -1);
	kprintf("file opened. fd: %d\n", fd);

	// read
	n = read(fd, bufr, rd_bytes);
	assert(n == rd_bytes);
	bufr[n] = 0;
	kprintf("%d bytes read: %s\n", n, bufr);

	close(fd);

	char * filenames[] = {"/foo", "/bar", "/baz"};
	// create files
	for (i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++) {
		fd = open(filenames[i], O_CREAT | O_RDWR);
		assert(fd != -1);
		kprintf("File created: %s (fd %d)\n", filenames[i], fd);
		close(fd);
	}
	char * rfilenames[] = {"/bar", "/foo", "/baz", "/dev_tty0"};

	// remove files
	for (i = 0; i < sizeof(rfilenames) / sizeof(rfilenames[0]); i++) {
		if (unlink(rfilenames[i]) == 0)
			kprintf("File removed: %s\n", rfilenames[i]);
		else
			kprintf("Failed to remove file: %s\n", rfilenames[i]);
	}
	
	kspin("test a");	
}

void test_b(){
	kspin("test_b");
	char tty_name[] = "/dev_tty1";
	kprintf("before open\n");	
	int fd_stdin = open(tty_name, O_RDWR);
	kprintf("open tty ok\n");
	assert(fd_stdin == 0);
	kspin("end test b here");

	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);
	kprintf("open tty ok2");

	char rdbuf[128];
	
	while(1){
		kprintf("$ ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;

		if(strcmp(rdbuf, "hello") == 0){
			kprintf("hello world!\n");
		}else{
			if(rdbuf[0]){
				kprintf("{%s}\n", rdbuf);
			}
		}
	}

	assert(0); // never arrive here.*/
}

void test_c(){
	kspin("test_c");
	kprintf(">>>getpid: %d\n", getpid());
	int i = 0x2000;
	while(1){	
		//printf(".");
		//printf("<C: %d>", i++);
		delay(5000);
	}
}