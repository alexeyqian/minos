#include "test.h"
#include "ipc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "assert.h"
#include "string.h"
#include "klib.h"
#include "kio.h"

void test_delay(){
    while(1){			
		kprintf(">>> ticks: %d\n", get_ticks());
		delay(5000);
	}
}

void test_fs(){
    int fd, n, i;
	const char filename[] = "blah";
	const char bufw[] = "abcde";
	const int rd_bytes = 3;
	char bufr[rd_bytes];
	kassert(rd_bytes <= strlen(bufw));
	// create
	fd = open(filename, O_CREAT | O_RDWR);
	kprintf("opened, fd: %d", fd);
	kassert(fd != -1);
	kprintf("File created. %s, fd: %d\n", filename, fd);
	
	// write
	n = write(fd, bufw, strlen(bufw));
	kassert(n == strlen(bufw));

	close(fd);
	
	// open
	fd = open(filename, O_RDWR);
	kassert(fd != -1);
	kprintf("file opened. fd: %d\n", fd);

	// read
	n = read(fd, bufr, rd_bytes);
	kassert(n == rd_bytes);
	bufr[n] = 0;
	kprintf("%d bytes read: %s\n", n, bufr);

	close(fd);

	char * filenames[] = {"/foo", "/bar", "/baz"};
	// create files
	for (i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++) {
		fd = open(filenames[i], O_CREAT | O_RDWR);
		kassert(fd != -1);
		kprintf("File created: %s (fd %d)\n", filenames[i], fd);
		close(fd);
	}
	char * rfilenames[] = {"/foo", "/bar", "/baz", "/dev_tty0"};

	// remove files
	for (i = 0; i < sizeof(rfilenames) / sizeof(rfilenames[0]); i++) {
		if (unlink(rfilenames[i]) == 0)
			kprintf("File removed: %s\n", rfilenames[i]);
		else
			kprintf("Failed to remove file: %s\n", rfilenames[i]);
	}	
}

void test_fs_tty(){
    char tty_name[] = "/dev_tty0";
	int fd_stdin = open(tty_name, O_RDWR);
	kprintf("df_stdin: %d\n", fd_stdin);
	kassert(fd_stdin == 0);
	
	int fd_stdout = open(tty_name, O_RDWR);
	kprintf("df_stdout: %d\n", fd_stdout);
	kassert(fd_stdout == 1);
		
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

	kassert(0); // never arrive here.*/
}

// <ring 1>, "system calls" via IPC message
PUBLIC void task_test(){
    kprintf(">>> 5. task_test is running\n"); 
    //test_fs();
    //test_delay();
    test_fs_tty();

    kspin("task_test");
}


