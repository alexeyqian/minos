all: os-image

# This is the actual disk image that the computer loads
# which is the combination of our compiled bootsector and kernel
os-image: boot/boot_sect.bin kernel/kernel.bin
	cat $^ > $@

boot/boot_sect.bin: boot/boot_sect.asm
	nasm $< -f bin -I 'boot/' -o $@

kernel/kernel.bin: kernel/kernel_entry.o kernel/kernel.o
	ld -o $@ -Ttext 0x1000 $^ --oformat binary

kernel/kernel_entry.o: kernel/kernel_entry.asm
	nasm $< -f elf -I 'kernel/' -o $@

kernel/kernel.o: kernel/kernel.c
	gcc -ffreestanding -c $< -o $@

clean:	
	@rm -rf kernel/*.o kernel/*.bin boot/*.o boot/*.bin drivers/*.o drivers/*.bin