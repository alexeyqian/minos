C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)
OBJ = $(wildcard kernel/*.o drivers/*.o)

all: os-image

# This is the actual disk image that the computer loads
# which is the combination of our compiled bootsector and kernel
os-image: boot/boot_sect.bin kernel/kernel.bin
	cat $^ > $@

boot/boot_sect.bin: boot/boot_sect.asm
	nasm $< -f bin -I 'boot/' -o $@

kernel/kernel.bin: kernel/kernel_entry.o ${OBJ} drivers/screen.o
	ld -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

kernel/kernel_entry.o: kernel/kernel_entry.asm ${OBJ}
	nasm $< -f elf32 -I 'kernel/' -o $@

#kernel/kernel.o: kernel/kernel.c
#	gcc -m32 -ffreestanding -fno-pie -g -c $< -o $@
%.o: %.c ${HEADERS}
	gcc -m32 -ffreestanding -fno-pie -c $< -o $@

clean:	
	@rm -rf kernel/*.o kernel/*.bin boot/*.o boot/*.bin drivers/*.o drivers/*.bin