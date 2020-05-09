C_SOURCES=$(wildcard kernel/*.c)
HEADERS=$(wildcard kernel/*.h)
OBJ=$(C_SOURCES:.c=.o)
#$(info $$C_SOURCES is [${C_SOURCES}])
#$(info $$OBJ is [${OBJ}])

all: os-image

run: all
	"/mnt/c/Program FIles/Bochs-2.6.11/bochs.exe"
os-image: boot/boot1.bin
	cat $^ > $@

#os-image: boot/boot.bin kernel/kernel.bin
#	cat $^ > $@

boot/boot.bin: boot/boot1.bin boot/boot2.bin
	cat $^ > $@

boot/boot1.bin: boot/boot1.asm
	nasm $< -f bin -I 'boot/' -o $@

boot/boot2.bin: boot/boot2.asm
	nasm $< -f bin -I 'boot/' -o $@

kernel/kernel.bin: kernel/kernel_entry.o kernel/kernel.o ${OBJ}
	ld -m elf_i386 -Ttext 0x1000 --oformat binary -o $@  $^ 

kernel/kernel_entry.o: kernel/kernel_entry.asm
	nasm $< -f elf32 -I 'kernel/' -o $@

%.o: %.c ${HEADERS}
	gcc -m32 -ffreestanding -fno-pie -c $< -o $@

clean:	
	@rm -rf os-image kernel/*.o kernel/*.bin boot/*.o boot/*.bin drivers/*.o