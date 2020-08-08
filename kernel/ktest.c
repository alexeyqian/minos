#include "ktest.h"
#include "clock.h"
#include "kio.h"
#include "fs.h"
#include "assert.h"
#include "string.h"

void test_a(){	
	printl(">>> running in test_a\n");
	spin("spin in test_a()");
	int fd, n, i;
	const char filename[] = "blah";
	const char bufw[] = "abcde";
	const int rd_bytes = 3;
	char bufr[rd_bytes];

	assert(rd_bytes <= strlen(bufw));

	// create
	printl("before open");
	fd = open(filename, O_CREAT | O_RDWR);
	printl("after open");
	assert(fd != -1);
	printl("File created. fd: %d\n", fd);
	
	// write
	printl("before write");
	n = write(fd, bufw, strlen(bufw));
	printl("after write");
	assert(n == strlen(bufw));

	close(fd);
	printl("after close");
	
	// open
	printl("before reopen");
	fd = open(filename, O_RDWR);
	printl("after reopen");
	assert(fd != -1);
	printl("file opened. fd: %d\n", fd);

	// read
	n = read(fd, bufr, rd_bytes);
	printl("after read");
	assert(n == rd_bytes);
	bufr[n] = 0;
	printl("%d bytes read: %s\n", n, bufr);

	close(fd);
	printl("after reclose");

	char * filenames[] = {"/foo", "/bar", "/baz"};
	// create files
	for (i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++) {
		fd = open(filenames[i], O_CREAT | O_RDWR);
		assert(fd != -1);
		printl("File created: %s (fd %d)\n", filenames[i], fd);
		close(fd);
	}
	printl("after create files");
	char * rfilenames[] = {"/bar", "/foo", "/baz", "/dev_tty0"};

	// remove files
	for (i = 0; i < sizeof(rfilenames) / sizeof(rfilenames[0]); i++) {
		if (unlink(rfilenames[i]) == 0)
			printl("File removed: %s\n", rfilenames[i]);
		else
			printl("Failed to remove file: %s\n", rfilenames[i]);
	}
	
	spin("test a");	
}

void test_b(){
	printl(">>> running in test_b\n");
	
	//spin("spin in test_b()");
	char tty_name[] = "/dev_tty1";
	printl("before open\n");	
	int fd_stdin = open(tty_name, O_RDWR);
	printl("open tty ok\n");
	assert(fd_stdin == 0);
	spin("end test b here");

	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);
	printl("open tty ok2");

	char rdbuf[128];
	
	while(1){
		printf("$ ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;

		if(strcmp(rdbuf, "hello") == 0){
			printf("hello world!\n");
		}else{
			if(rdbuf[0]){
				printf("{%s}\n", rdbuf);
			}
		}
	}

	assert(0); // never arrive here.*/
}

void test_c(){
	int i = 0x2000;
	while(1){	
		//printl(".");
		//printl("<C: %d>", i++);
		delay(5000);
	}
}