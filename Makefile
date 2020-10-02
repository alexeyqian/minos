CROSS_COMPILER=i686-elf-gcc
C_SOURCES=$(wildcard kernel/*.c)
C_HEADERS=$(wildcard include/*.h)
C_FLAGS = -c -std=gnu99 -ffreestanding -O0 -Wall -Wcast-align -Wextra -I include/
C_OBJS=$(C_SOURCES:.c=.o)
L_FLAGS = -T linker.ld -ffreestanding -nostdlib

all: clean os.img

clean:	
	@rm -rf os.img boot/*.o boot/*.bin kernel/*.o kernel/*.bin \
	lib/*.o lib/*.a drivers/hd/*.o drivers/hd/*.bin services/fs/*.o services/fs/*.a

dev: tty/tty_main.c
	$(CROSS_COMPILER) $(C_FLAGS) -Wsign-compare -Wconversion -o \
	tty/tty_main.o tty/tty_main.c	

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

kernel/kernel.bin: $(C_SOURCES) $(C_HEADERS) lib/vsprintf.c
	nasm kernel/asm/kernel_entry.asm -f elf32 -I 'kernel/asm/' -o kernel/asm/kernel_entry.o	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/main.o kernel/main.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/kprintf.o kernel/kprintf.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/global.o kernel/global.c		
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/boot_params.o kernel/boot_params.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/interrupt.o kernel/interrupt.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/protect.o kernel/protect.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/phys_mem.o kernel/phys_mem.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/virt_mem.o kernel/virt_mem.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/kcall.o kernel/kcall.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/ipc.o kernel/ipc.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/proc.o kernel/proc.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/clock.o kernel/clock.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o kernel/system.o kernel/system.c		
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/string.o lib/string.c
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/vsprintf.o lib/vsprintf.c
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/printx.o lib/printx.c
	$(CROSS_COMPILER) $(C_FLAGS) -o lib/misc.o lib/misc.c
	$(CROSS_COMPILER) $(C_FLAGS) -o drivers/hd/hd.o drivers/hd/hd.c	
	$(CROSS_COMPILER) $(C_FLAGS) -o services/fs/fs.o services/fs/fs.c	
	$(CROSS_COMPILER) $(L_FLAGS) -o $@ kernel/asm/kernel_entry.o $(C_OBJS) \
		lib/vsprintf.o lib/string.o lib/printx.o lib/misc.o drivers/hd/hd.o services/fs/fs.o -lgcc	
