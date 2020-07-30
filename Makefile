CROSS_COMPILER=i686-elf-gcc
C_SOURCES=$(wildcard kernel/*.c)
C_HEADERS=$(wildcard include/*.h)
#C_FLAGS = -c -m32 -ffreestanding -fno-exceptions -I include/
C_FLAGS = -c -std=gnu99 -ffreestanding -O0 -Wall -Wcast-align -Wsign-compare -Wconversion -Wextra -I include/
#-fno-builtin -O0 or -O2?
C_OBJS=$(C_SOURCES:.c=.o)
#$(info $$C_SOURCES is [${C_SOURCES}])
#$(info $$OBJ is [${OBJ}])


all: clean os.img

#run: all
#

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

kernel/kernel.bin: $(C_SOURCES) $(C_HEADERS) \
		fs/fs_main.c fs/fs_open.c fs/fs_shared.c fs/fs_shared.h fs/fs_open.h
	nasm kernel/kernel_entry.asm -f elf32 -I 'kernel/' -o kernel/kernel_entry.o	
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
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/string.o    kernel/string.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/assert.o    kernel/assert.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/vsprintf.o  kernel/vsprintf.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/kio.o       kernel/kio.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/tty.o       kernel/tty.c
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/ktest.o     kernel/ktest.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/ipc.o       kernel/ipc.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/hd.o        kernel/hd.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/kernel.o    kernel/kernel.c
	$(CROSS_COMPILER) $(C_FLAGS) -o fs/fs_main.o      fs/fs_main.c
	$(CROSS_COMPILER) $(C_FLAGS) -o fs/fs_open.o      fs/fs_open.c
	$(CROSS_COMPILER) $(C_FLAGS) -o fs/fs_shared.o    fs/fs_shared.c
	#$(CROSS_COMPILER) -T linker.ld -o $@ -ffreestanding -nostdlib kernel/kernel_entry.o $(C_OBJS) fs.o -lgcc
	ld -m elf_i386 -s -Ttext 0x30400 -nostdlib -o $@ kernel/kernel_entry.o $(C_OBJS) \
		fs/fs_main.o fs/fs_open.o fs/fs_shared.o

#kernel/kernel.bin: kernel/kernel_entry.o kernel/kernel.o ${OBJ}
#	ld -m elf_i386 -Ttext 0x1000 --oformat binary -o $@  $^ 

#kernel/kernel_entry.o: kernel/kernel_entry.asm
#	nasm $< -f elf32 -I 'kernel/' -o $@

#%.o: %.c ${HEADERS}
#	gcc -m32 -ffreestanding -fno-pie -c $< -o $@

clean:	
	@rm -rf os.img boot/*.o boot/*.bin kernel/*.o kernel/*.bin fs/*.o