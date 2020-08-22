# minos
Minimal OS Kernel, inspired by:
- Yuan Yu (Tinix)
- Andrew S. Tanenbaum (Minix3)
- http://www.brokenthorn.com/Resources/OSDevIndex.html
- osdev.org

The best way to study OS is to make a simple one by yourself.

# Compiler
Create your own cross platfom gcc compiler: i686-elf-gcc, the hosted compiler is not working at this stage.
Why do I need a cross compiler: https://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F
implement your own memcmp, memcpy, memmove, memset and abort.
int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
# Bochs debug commands
lb 0x090100 / info break / info cpu / s <n> / c / regs / sregs / cregs /

# Tools to check binary file
od -t x1 -A n boot/boot_sect.bin
hp -C filename
## test code
if<condition>
    kprintf("arrive here:: x");
    while(1){}

# Boot Loader
It's a two stage boot loader, (not yet)) follow multiboot specification.
## Boot and loder info and error messages
The reason for just print a leter on top left of screen to show the steps instead of meaningful message is to save size,
since we only have 512 bytes in boot.
- A: loader.bin file found in boot
- B: loader.bin is loaded into memory successfully in boot
- C: start run in loader
- D: got memory map
- E: kernel.bin found in loader.
- F: kernel.bin is loaded in memory 
- G: start running in protect mode
- 1: loader.bin file is not found in boot.
- 2: kenel.bin not found in loader
- 3
## First Stage: Boot
It's the boot sector on block on floppy disk/hard disk, which is the first block contains 512 bytes.
It includes the 0xaa55 at locataion 0x510 and 0x511, the magic number which makes BIOS think it's bootable.
This first 512 bytes on disk, only includes code/functions to load 'loader.bin' from disk to memory, 
and transfer control to loader.bin.
boot.asm is compiled to 'bin' format, which is flat binary without any format/metadata.
it read some disk sectors, and parsing fat12 disk format to load add data sector in floppy disk for file loader.bin
### Stack info
- SS: 0x0
- BP and SP: 0x7c00

## Second Stage: Load
loader.asm, compiled to bin format.
Loader.bin is Loaded by boot at address: 0x90000, entry point is at: 0x90000.
loader statck is located at: enf of loader.bin in memory + 1K
Start to prepare everything for kernel, then transfer control to kernel entry point.
### Stack Info
- Real Mode: SS: 0x9000, BP and SP: end of loader + 1K
- Protected Mode: SS: data_selector, EBP and ESP: end of loader.bin in memory + 1K

so in total: loader is using: size of loader.bin + 1K stack space in memory

It prepares:
- get memory size
- get memory map
- fill kernel info structure
- load file kernel.bin into memory.
- entery protected mode
- transfer control to kernel

### main purpose: loading loader.bin to memory
loader.bin loaded into 0x90000 ~ 0x9fc00
### Read Data from Disk Sectors

### Parse FAT12 File System

### Load the Loader

## Loader

### Main Purpose: loading kernel.bin to memory

### Load Kernel Image into Memory from Disk 
kernel.bin loaded to 0x70000 - 0x90000 (128K)
Another way to avoid loading temp kernel.bin file is to use
tool to extract all binaray from elf formated kernel.bin file
and create a pure binary file, so it would not need this tempary step.
### Enter Protected Mode

#### Setup GDT and IDT

### Parsing ELF File Format

### Load Kernel into Memory
loaded into 0x1000h - 0x6ffff (450K)
mem range 0x500 - 0x1000 - 1 is used to pass params from loader to kernel

### Transfer Control to Kernel

# Kernel
; kernel is first read to at address:0x70000 from disk by loader in real mode 
; then it's been moved to address:0x1000 from 0x70000 to execute in protection mode.
; the reason for read first then move, is because is easy to read from disk in real mode which has BIOS.
; but the kernel is elf format, and cannot run directly as pure bin, since it has format data.
; so it need to parse the elf file and load/move 'segments' into memory, so it can execute. 
; in protected mode, read from disk is hard, but read from memory is easy.
; so read the file data from disk to memory as buffer for disk data, then parse the data in memory and prepare for execution.

For file: kernel.c written in c and compiled to elf format, which already include stack segments, so no need to manually assign stack ebp and esp.
For file: kernel_entry.asm, also compiled to elf format, but it's using asm, so it still need to manually set stack ebp and esp.
## the reason for entry point address: 30400
In future, might need to find a better solution to remove this 400 thing, to make it simple.
?? Since default is 0x8004800 which is above 128M, so need to low it to under 1M
?? need reseserve some space (0x400) for elf header and program headers/section headers.
?? above statements are my guess, might wrong, need to confirm in future.
## Kernel Initialize Process
- replace gdt from loader to kernel.
- setup idt
- setup clock handler
- setup proc array for system tasks and user processes.
### Switch from Loader's GDT to Kernel's GDT

### Init IDT
- Init 8159A chip
- Init IDT items

### Init TSS in GDT

### Init LDT descriptors in GDT

### Init Process Table (includes PCB items)

### Task Switching
Process switch must be done in a shared area, usally provided by kernel.
Thread switch can be done in application area.

The TSS is primarily suited for hardware multitasking, where each individual process has its own TSS. In Software multitasking, one or two TSS's are also generally enough, as they allow for entering Ring 0 code after an interrupt.

There are a lot of subtle things with user mode and task switching that you may not realize at first. First: Whenever a system call interrupt happens, the first thing that happens is the CPU changes to ESP0 stack. Then, it will push all the system information. So when you enter the interrupt handler, your working off of the ESP0 stack. This could become a problem with 2 ring 3 tasks going if all you do is merely push context info and change esp. Think about it. you will change the esp, which is the esp0 stack, to the other tasks esp, which is the same esp0 stack. So, what you must do is change the ESP0 stack(along with the interrupt pushed ESP stack) on each task switch, or you'll end up overwriting yourself.

Whenever a system call occurs, the CPU gets the SS0 and ESP0-value in its TSS and assigns the stack-pointer to it. So one or more kernel-stacks need to be set up for processes doing system calls. Be aware that a thread's/process' time-slice may end during a system call, passing control to another thread/process which may as well perform a system call, ending up in the same stack. Solutions are to *create a private kernel-stack for each thread/process* and re-assign esp0 at any task-switch or to disable scheduling during a system-call.


instruction iret can detect itself if the switch is involving privillege change or not.

### Restart: Starting User Processes

## Exceptions and Interrupts Handling

Inter CPU only has one externel pin: INTR for external interrupt, for accept signal from
multiple external devices, it use PIC to collect all external requests and filter them, then select one deliver to the pin:INTR.

NMI is not exception, it's a external not maskable interrupt.
External interrupt and instruction: INT n both will not generate error code.

When exception happens, CR2 will store the linear address, which can be used to locate the page directory table and page table, and the exception handler should save the value of CR2 before the next page fault exception.
## Locking

## Memory Management
### Physical Memory Management
Loader will store memory segmentation info in physical address: 0x500 during loading stage.
And kernel can read that info out.

The real managable physical memory has to be aligned based on the page size: 4K or 2M.

virt2phys() used to convert virtual address to physical address in kernel 

### Virtual Memory Management

# Process Management
# Task Switch
# Process Schedule
# Process Structure
## PCB process control block




# File System
## Hard drive driver

## File System Layers


## Superblock


## Meta Data

## Data Storage


## Memory Buffer of File System

## Read and Write
