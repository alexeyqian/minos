#include "_init.h"

struct posix_tar_header
{				/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
	/* 500 */
};

void untar(const char* filename){
	printx(">>> extract %s\n", filename);
	int fd = open(filename, O_RDWR);
	assertx(fd != -1);

	char buf[SECTOR_SIZE*16];
	int chunk = sizeof(buf);
	while(1){
		read(fd, buf, SECTOR_SIZE);
		if(buf[0] == 0) break;

		struct posix_tar_header* phdr = (struct posix_tar_header*)buf;

		// calculate file size
		char* p = phdr->size;
		int f_len = 0;
		while(*p)
			f_len = (f_len*8) + (*p++ - '0'); //octal
		
		int bytes_left = f_len;
		int fdout = open(phdr->name, O_CREAT|O_RDWR);
		if(fdout == -1){
			printx("- failed to extract file: %s, aborted!\n", phdr->name);
			return;
		}

		printx("- %s(%d bytes\n", phdr->name, f_len);
		while(bytes_left){
			int iobytes = min(chunk, bytes_left);
			read(fd, buf, ((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			write(fdout, buf, iobytes);
			bytes_left -= iobytes;
		}
		close(fdout);
	}

	close(fd);
	printx("extract done.\n");
}

void shabby_shell(const char * tty_name)
{
	int fd_stdin  = open(tty_name, O_RDWR);
	assertx(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assertx(fd_stdout == 1);

	char rdbuf[128];

	while (TRUE) {
		write(1, "$ ", 2);
		int r = read(0, rdbuf, 70);
		rdbuf[r] = 0;

		int argc = 0;
		char * argv[PROC_ORIGIN_STACK];
		char * p = rdbuf;
		char * s;
		int word = 0;
		char ch;
		do {
			ch = *p;
			if (*p != ' ' && *p != 0 && !word) {
				s = p;
				word = 1;
			}
			if ((*p == ' ' || *p == 0) && word) {
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		} while(ch);
		argv[argc] = 0;

		int fd = open(argv[0], O_RDWR);
		if (fd == -1) {
			if (rdbuf[0]) {
				write(1, "{", 1);
				write(1, rdbuf, r);
				write(1, "}\n", 2);
			}
		}
		else {
			close(fd);
			int pid = fork();
			if (pid != 0) { // parent 
				int s;
				wait(&s);
			}
			else {	// child 
				execv(argv[0], argv);
			}
		}
	}

	close(1);
	close(0);
}

// first user process, parent for all user processes.
PUBLIC void init(){
	printx(">>> 6. init is running.\n");
	int fd_stdin = open("/dev_tty0", O_RDWR);
	assertx(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assertx(fd_stdout == 1);

	untar("/inst.tar");
	//clear_screen(); 

	// from here, the first user process init is started.
	// after this point, we can use printf now.

	char* tty_list[] = {"/dev_tty1", "/dev_tty2"};
	for(uint32_t i = 0; i < sizeof(tty_list)/sizeof(tty_list[0]); i++){
		int pid = fork();
		if (pid != 0) { // parent process
			printx("[parent is running, child pid:%d]\n", pid);
		}
		else {	// child process
			printx("[child is running, pid:%d]\n", getpid());
			close(fd_stdin);
			close(fd_stdout);
			
			shabby_shell(tty_list[i]);
			panic("should never be here!");
		}
	}

	// keep wating for other process to exist as transferred parents.
	while(TRUE){
		int s;
		int child = wait(&s);
		printx("[Init] child %d exited with satus: %d\n", child, s);
	}
    panic("should never be here!");
}