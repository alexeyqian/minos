C_SOURCES=$(wildcard kernel/*.c)
HEADERS=$(wildcard kernel/*.h)
OBJ=$(C_SOURCES:.c=.o)
#$(info $$C_SOURCES is [${C_SOURCES}])
#$(info $$OBJ is [${OBJ}])
CFLAGS = -c -m32 -fno-builtin -I include/
all: clean os.img

#run: all
#

os.img: boot/boot.bin boot/loader.bin kernel/kernel.bin
	dd if=/dev/zero of=os.img bs=512 count=2880
	mkfs.msdos os.img
	dd if=boot/boot.bin of=os.img bs=512 count=1 conv=notrunc
	mount -oloop os.img /mnt/floppy
	cp boot/loader.bin /mnt/floppy
	cp kernel/kernel.bin /mnt/floppy
	umount /mnt/floppy

boot/boot.bin: boot/boot.asm
	nasm $< -f bin -I 'boot/' -o $@

boot/loader.bin: boot/loader.asm
	nasm $< -f bin -I 'boot/' -o $@

kernel/kernel.bin: kernel/kernel_entry.asm kernel/klib.c kernel/global.c kernel/interrupt.c kernel/proc.c \
	kernel/clock.c kernel/phys_mem.c kernel/virt_mem.c \
	kernel/screen.c kernel/keyboard.c kernel/tty.c kernel/ktest.c kernel/ipc.c  kernel/kernel.c
	nasm kernel/kernel_entry.asm -f elf32 -I 'kernel/' -o kernel/kernel_entry.o	
	gcc $(CFLAGS) -o kernel/klib.o      kernel/klib.c
	gcc $(CFLAGS) -o kernel/global.o    kernel/global.c
	gcc $(CFLAGS) -o kernel/interrupt.o kernel/interrupt.c
	gcc $(CFLAGS) -o kernel/proc.o      kernel/proc.c
	gcc $(CFLAGS) -o kernel/clock.o     kernel/clock.c
	gcc $(CFLAGS) -o kernel/phys_mem.o  kernel/phys_mem.c
	gcc $(CFLAGS) -o kernel/virt_mem.o  kernel/virt_mem.c
	gcc $(CFLAGS) -o kernel/keyboard.o  kernel/keyboard.c
	gcc $(CFLAGS) -o kernel/screen.o    kernel/screen.c
	gcc $(CFLAGS) -o kernel/tty.o       kernel/tty.c
	gcc $(CFLAGS) -o kernel/ktest.o     kernel/ktest.c	
	gcc $(CFLAGS) -o kernel/ipc.o       kernel/ipc.c	
	gcc $(CFLAGS) -o kernel/kernel.o    kernel/kernel.c
	ld -m elf_i386 -s -Ttext 0x30400 -o $@ kernel/kernel_entry.o kernel/klib.o \
	kernel/global.o kernel/interrupt.o kernel/proc.o kernel/clock.o \
	kernel/phys_mem.o kernel/virt_mem.o kernel/screen.o kernel/keyboard.o \
	kernel/tty.o kernel/ktest.o kernel/ipc.o kernel/kernel.o
	
#kernel/kernel.bin: kernel/kernel_entry.o kernel/kernel.o ${OBJ}
#	ld -m elf_i386 -Ttext 0x1000 --oformat binary -o $@  $^ 

#kernel/kernel_entry.o: kernel/kernel_entry.asm
#	nasm $< -f elf32 -I 'kernel/' -o $@

#%.o: %.c ${HEADERS}
#	gcc -m32 -ffreestanding -fno-pie -c $< -o $@

clean:	
	@rm -rf os.img boot/*.o boot/*.bin kernel/*.o kernel/*.bin