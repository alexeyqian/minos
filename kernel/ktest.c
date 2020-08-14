#include "ktest.h"
#include "clock.h"
#include "kio.h"
#include "fs.h"
#include "assert.h"
#include "string.h"

void test_a(){	
	printf(">>> running in test_a\n");
	spin("spin in test_a()");
	int fd, n, i;
	const char filename[] = "blah";
	const char bufw[] = "abcde";
	const int rd_bytes = 3;
	char bufr[rd_bytes];

	assert(rd_bytes <= strlen(bufw));

	// create
	printf("before open");
	fd = open(filename, O_CREAT | O_RDWR);
	printf("after open");
	assert(fd != -1);
	printf("File created. fd: %d\n", fd);
	
	// write
	printf("before write");
	n = write(fd, bufw, strlen(bufw));
	printf("after write");
	assert(n == strlen(bufw));

	close(fd);
	printf("after close");
	
	// open
	printf("before reopen");
	fd = open(filename, O_RDWR);
	printf("after reopen");
	assert(fd != -1);
	printf("file opened. fd: %d\n", fd);

	// read
	n = read(fd, bufr, rd_bytes);
	printf("after read");
	assert(n == rd_bytes);
	bufr[n] = 0;
	printf("%d bytes read: %s\n", n, bufr);

	close(fd);
	printf("after reclose");

	char * filenames[] = {"/foo", "/bar", "/baz"};
	// create files
	for (i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++) {
		fd = open(filenames[i], O_CREAT | O_RDWR);
		assert(fd != -1);
		printf("File created: %s (fd %d)\n", filenames[i], fd);
		close(fd);
	}
	printf("after create files");
	char * rfilenames[] = {"/bar", "/foo", "/baz", "/dev_tty0"};

	// remove files
	for (i = 0; i < sizeof(rfilenames) / sizeof(rfilenames[0]); i++) {
		if (unlink(rfilenames[i]) == 0)
			printf("File removed: %s\n", rfilenames[i]);
		else
			printf("Failed to remove file: %s\n", rfilenames[i]);
	}
	
	spin("test a");	
}

void test_b(){
	printf(">>> running in test_b\n");
	
	spin("spin in test_b()");
	char tty_name[] = "/dev_tty1";
	printf("before open\n");	
	int fd_stdin = open(tty_name, O_RDWR);
	printf("open tty ok\n");
	assert(fd_stdin == 0);
	spin("end test b here");

	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);
	printf("open tty ok2");

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
		//printf(".");
		//printf("<C: %d>", i++);
		delay(5000);
	}
}