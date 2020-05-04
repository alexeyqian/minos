C_SOURCES=$(wildcard kernel/*.c)
HEADERS=$(wildcard kernel/*.h)
OBJ=$(C_SOURCES:.c=.o)
$(info $$C_SOURCES is [${C_SOURCES}])
$(info $$OBJ is [${OBJ}])

all: os-image

run: all
	"/mnt/c/Program FIles/Bochs-2.6.11/bochs.exe"

# This is the actual disk image that the computer loads
# which is the combination of our compiled bootsector and kernel
os-image: boot/boot_sect.bin kernel/kernel.bin
	cat $^ > $@

boot/boot_sect.bin: boot/boot_sect.asm
	nasm $< -f bin -I 'boot/' -o $@

kernel/kernel.bin: kernel/kernel_entry.o kernel/kernel.o ${OBJ}
	ld -m elf_i386 -Ttext 0x1000 --oformat binary -o $@  $^ 

kernel/kernel_entry.o: kernel/kernel_entry.asm
	nasm $< -f elf32 -I 'kernel/' -o $@

%.o: %.c ${HEADERS}
	gcc -m32 -ffreestanding -fno-pie -c $< -o $@

clean:	
	@rm -rf os-image kernel/*.o kernel/*.bin boot/*.o boot/*.bin drivers/*.o