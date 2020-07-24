C_SOURCES=$(wildcard kernel/*.c)
C_HEADERS=$(wildcard include/*.h)
C_FLAGS = -c -m32 -fno-builtin -I include/
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
	mount -oloop os.img /mnt/floppy
	cp boot/loader.bin /mnt/floppy
	cp kernel/kernel.bin /mnt/floppy
	umount /mnt/floppy

boot/boot.bin: boot/boot.asm
	nasm $< -f bin -I 'boot/' -o $@

boot/loader.bin: boot/loader.asm
	nasm $< -f bin -I 'boot/' -o $@

kernel/kernel.bin: $(C_SOURCES) $(C_HEADERS)
	nasm kernel/kernel_entry.asm -f elf32 -I 'kernel/' -o kernel/kernel_entry.o	
	gcc $(C_FLAGS) -o kernel/klib.o      kernel/klib.c
	gcc $(C_FLAGS) -o kernel/global.o    kernel/global.c
	gcc $(C_FLAGS) -o kernel/interrupt.o kernel/interrupt.c
	gcc $(C_FLAGS) -o kernel/proc.o      kernel/proc.c
	gcc $(C_FLAGS) -o kernel/clock.o     kernel/clock.c
	gcc $(C_FLAGS) -o kernel/memory.o    kernel/memory.c
	gcc $(C_FLAGS) -o kernel/phys_mem.o  kernel/phys_mem.c
	gcc $(C_FLAGS) -o kernel/virt_mem.o  kernel/virt_mem.c
	gcc $(C_FLAGS) -o kernel/keyboard.o  kernel/keyboard.c
	gcc $(C_FLAGS) -o kernel/screen.o    kernel/screen.c
	gcc $(C_FLAGS) -o kernel/string.o    kernel/string.c
	gcc $(C_FLAGS) -o kernel/assert.o    kernel/assert.c
	gcc $(C_FLAGS) -o kernel/vsprintf.o  kernel/vsprintf.c
	gcc $(C_FLAGS) -o kernel/kio.o       kernel/kio.c
	gcc $(C_FLAGS) -o kernel/tty.o       kernel/tty.c
	gcc $(C_FLAGS) -o kernel/ktest.o     kernel/ktest.c	
	gcc $(C_FLAGS) -o kernel/ipc.o       kernel/ipc.c	
	gcc $(C_FLAGS) -o kernel/hd.o        kernel/hd.c	
	gcc $(C_FLAGS) -o kernel/kernel.o    kernel/kernel.c
	ld -m elf_i386 -s -Ttext 0x30400 -o $@ kernel/kernel_entry.o $(C_OBJS)
	
#kernel/kernel.bin: kernel/kernel_entry.o kernel/kernel.o ${OBJ}
#	ld -m elf_i386 -Ttext 0x1000 --oformat binary -o $@  $^ 

#kernel/kernel_entry.o: kernel/kernel_entry.asm
#	nasm $< -f elf32 -I 'kernel/' -o $@

#%.o: %.c ${HEADERS}
#	gcc -m32 -ffreestanding -fno-pie -c $< -o $@

clean:	
	@rm -rf os.img boot/*.o boot/*.bin kernel/*.o kernel/*.bin