/* The bootloader will look at this image 
   and start execution at the symbol designated as the entry point. */
ENTRY(_start)
/* It is which file you want to be linked to the very beginning of the executable.*/
/*STARTUP(kernel/kernel_entry.o) */
/* Tell where the various sections of the object files will be put in the final kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB/(changed to ox30400), 
       a conventional place for kernels to be loaded at by the bootloader. */
	. = 0x1000;
 
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text BLOCK(4K) : ALIGN(4K)
	{
		/* no multiboot support yet *(.multiboot) */
		*(.text)
	}
 
	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
 
	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
 
	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}