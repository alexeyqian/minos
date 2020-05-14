C_SOURCES=$(wildcard kernel/*.c)
HEADERS=$(wildcard kernel/*.h)
OBJ=$(C_SOURCES:.c=.o)
#$(info $$C_SOURCES is [${C_SOURCES}])
#$(info $$OBJ is [${OBJ}])

all: os.img

os.img: boot/boot.bin boot/loader.bin boot/kernel.bin
	dd if=/dev/zero of=os.img bs=512 count=2880
	mkfs.msdos os.img
	dd if=boot/boot.bin of=os.img bs=512 count=1 conv=notrunc
	mount -oloop os.img /mnt/floppy
	cp boot/loader.bin /mnt/floppy
	cp boot/kernel.bin /mnt/floppy
	umount /mnt/floppy

boot/boot.bin: boot/boot.asm
	nasm $< -f bin -I 'boot/' -o $@

boot/loader.bin: boot/loader.asm
	nasm $< -f bin -I 'boot/' -o $@

boot/kernel.bin: boot/kernel.asm boot/klib.asm boot/start.c
	nasm boot/kernel.asm -f elf32 -I 'boot/' -o boot/kernel.o
	nasm boot/klib.asm   -f elf32 -I 'boot/' -o boot/klib.o
	gcc -c -m32 -fno-builtin -o boot/start.o boot/start.c
	ld -m elf_i386 -s -Ttext 0x30400 -o $@ boot/kernel.o boot/klib.o boot/start.o

kernel/kernel.bin: kernel/kernel_entry.o kernel/kernel.o ${OBJ}
	ld -m elf_i386 -Ttext 0x1000 --oformat binary -o $@  $^ 

kernel/kernel_entry.o: kernel/kernel_entry.asm
	nasm $< -f elf32 -I 'kernel/' -o $@

%.o: %.c ${HEADERS}
	gcc -m32 -ffreestanding -fno-pie -c $< -o $@

clean:	
	@rm -rf os.img boot/*.o boot/*.bin kernel/*.o kernel/*.bin drivers/*.o