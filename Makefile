CROSS_COMPILER=i686-elf-gcc
C_SOURCES=$(wildcard kernel/*.c)
C_HEADERS=$(wildcard include/*.h)
#C_FLAGS = -c -m32 -ffreestanding -fno-exceptions -I include/
C_FLAGS = -c -std=gnu99 -ffreestanding -O0 -Wall -Wcast-align -Wextra -I include/
#-Wsign-compare -Wconversion
#-fno-builtin -O0 or -O2?
C_OBJS=$(C_SOURCES:.c=.o)
#$(info $$C_SOURCES is [${C_SOURCES}])
#$(info $$OBJ is [${OBJ}])

all: clean os.img

os.img: boot/boot.bin boot/loader.bin kernel/kernel.bin
	dd if=/dev/zero of=os.img bs=512 count=2880
	mkfs.msdos os.img
	dd if=boot/boot.bin of=os.img bs=512 count=1 conv=notrunc
	sudo mount -oloop os.img /mnt/floppy
	sudo cp boot/loader.bin /mnt/floppy
	sudo cp kernel/kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

boot/boot.bin: boot/boot.asm
	nasm $< -f bin -I 'boot/' -o $@

boot/loader.bin: boot/loader.asm
	nasm $< -f bin -I 'boot/' -o $@

refactory: kernel/keyboard.c
	$(CROSS_COMPILER) $(C_FLAGS) -Wsign-compare -Wconversion -o \
	kernel/keyboard.o kernel/keyboard.c	

kernel/kernel.bin: $(C_SOURCES) $(C_HEADERS) \
		fs/fs_main.c fs/fs_open.c fs/fs_shared.c fs/fs_shared.h fs/fs_open.h mm/mm_main.c 
	nasm kernel/kernel_entry.asm -f elf32 -I 'kernel/' -o kernel/kernel_entry.o	
	nasm lib/syscalls.asm -f elf32 -I 'lib/' -o lib/syscalls.o	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/boot_params.o kernel/boot_params.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/klib.o      kernel/klib.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/global.o    kernel/global.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/interrupt.o kernel/interrupt.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/proc.o      kernel/proc.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/clock.o     kernel/clock.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/memory.o    kernel/memory.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/phys_mem.o  kernel/phys_mem.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/virt_mem.o  kernel/virt_mem.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/keyboard.o  kernel/keyboard.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/screen.o    kernel/screen.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/ktest.o     kernel/ktest.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/ipc.o       kernel/ipc.c		
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/boot_params.o kernel/boot_params.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/kernel.o    kernel/kernel.c
	$(CROSS_COMPILER) $(C_FLAGS) -o syscall/syscall_main.o    syscall/syscall_main.c
	$(CROSS_COMPILER) $(C_FLAGS) -o hd/hd_main.o       hd/hd_main.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o fs/fs_main.o       fs/fs_main.c
	$(CROSS_COMPILER) $(C_FLAGS) -o fs/fs_open.o       fs/fs_open.c
	$(CROSS_COMPILER) $(C_FLAGS) -o fs/fs_shared.o     fs/fs_shared.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o tty/tty_main.o     tty/tty_main.c
	$(CROSS_COMPILER) $(C_FLAGS) -o mm/mm_main.o       mm/mm_main.c
	$(CROSS_COMPILER) $(C_FLAGS) -o test/test_main.o   test/test_main.c
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/vsprintf.o     lib/vsprintf.c
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/string.o       lib/string.c
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/assert.o       lib/assert.c		
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/stdio.o        lib/stdio.c		
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/ipclib.o       lib/ipclib.c		
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/fslib.o        lib/fslib.c		
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/proclib.o      lib/proclib.c		
	$(CROSS_COMPILER) -T linker.ld -o $@ -ffreestanding -nostdlib kernel/kernel_entry.o $(C_OBJS) \
		syscall/syscall_main.o hd/hd_main.o fs/fs_main.o fs/fs_open.o fs/fs_shared.o \
		tty/tty_main.o mm/mm_main.o test/test_main.o lib/vsprintf.o lib/string.o \
		lib/stdio.o lib/assert.o lib/ipclib.o lib/fslib.o lib/proclib.o lib/syscalls.o -lgcc	
	# minoscrt.a is runtime c lib, which should be independent, can only depend on syscalls	
	ar rcs lib/minoscrt.a lib/vsprintf.o lib/string.o lib/assert.o lib/stdio.o lib/ipclib.o lib/fslib.o lib/proclib.o lib/syscalls.o
	
tar:
	#inst.tar: command/start.asm command/echo.c	
	nasm -I include/ -f elf32 -o command/start.o command/start.asm
	gcc  -I include/ -m32 -c -fno-builtin -Wall -o command/echo.o command/echo.c 
	ld -Ttext 0x1000 -m elf_i386 -o echo command/echo.o command/start.o lib/minoscrt.a
	tar vcf inst.tar echo 
	# TODO: hard code ROOT_BASE INSTALL_START_SECT
	#seek=`echo "obase=10;ibase=16;(4EFF+8000)*200"|bc`
	dd if=inst.tar of=80m.img seek=27131392 bs=1 count=`ls -l inst.tar|awk -F " " '{print $$5}'` conv=notrunc

clean:	
	@rm -rf os.img inst.tar echo boot/*.o boot/*.bin kernel/*.o kernel/*.bin fs/*.o mm/*.o lib/*.o lib/*.a command/*.o

# old implementation
#ld -m elf_i386 -s -Ttext 0x1000 -nostdlib -o $@ kernel/kernel_entry.o $(C_OBJS) \
	#	syscall/syscall_main.o hd/hd_main.o fs/fs_main.o fs/fs_open.o fs/fs_shared.o \
	#	tty/tty_main.o mm/mm_main.o test/test_main.o lib/vsprintf.o lib/string.o \
	#	lib/stdio.o lib/assert.o lib/ipclib.o lib/fslib.o lib/proclib.o lib/syscalls.o


#kernel/kernel.bin: kernel/kernel_entry.o kernel/kernel.o ${OBJ}
#	ld -m elf_i386 -Ttext 0x1000 --oformat binary -o $@  $^ 

#kernel/kernel_entry.o: kernel/kernel_entry.asm
#	nasm $< -f elf32 -I 'kernel/' -o $@

#%.o: %.c ${HEADERS}
#	gcc -m32 -ffreestanding -fno-pie -c $< -o $@