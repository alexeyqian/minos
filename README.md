# minos
Minimal OS Kernel, inspired by Yuan Yu (Tinix) and Andrew S. Tanenbaum (Minix3).

The best way to study OS is to make a simple one by yourself.

# Boot Loader
It's a two stage boot loader, also follow multiboot specification.
## First Stage: Boot
It's the boot sector on block on floppy disk/hard disk, which is the first block contains 512 bytes.
It includes the 0xaa55 at locataion 0x510 and 0x511, the magic number which makes BIOS think it's bootable.
This first 512 bytes on disk, only includes code/functions to load 'loader.bin' from disk to memory, 
and transfer control to loader.bin.
boot.asm is compiled to 'bin' format, which is flat binary without any format/metadata.
it read some disk sectors, and parsing fat12 disk format to load add data sector in floppy disk for file loader.bin

## Second Stage: Load
loader.asm, compiled to bin format.
Loader.bin is Loaded by boot at address: 0x90100, entry point is at: 0x90100.
the address range: 0x90099 down to 0x90000 (0x100, 256bytes) is reserved as loader's stack space.
loader's ebp/esp points to 0x90099 and grow down to 0x90000.
Start to prepare everything for kernel, then transfer control to kernel entry point.

so in total: loader is using: loader.bin (start from 0x90100 + size) + loader stack space (256 bytes)
It prepares:
- get memory size
- get memory map
- fill kernel info structure
- load file kernel.bin into memory.
- entery protected mode
- transfer control to kernel

2 stacks for loader:
- first stack is 0x90100 - 0x9000 as stack in real mode
- second stack is end_of_loader + 4K - end_of_loader in protected mode

### main purpose: loading loader.bin to memory

### Read Data from Disk Sectors

### Parse FAT12 File System

### Load the Loader

## Loader

### Main Purpose: loading kernel.bin to memory

### Load Kernel Image into Memory from Disk 

### Enter Protected Mode

#### Setup GDT and IDT

### Parsing ELF File Format

### Load Kernel into Memory

### Transfer Control to Kernel

# Kernel

## Kernel Initialize Process
### Switch from Loader's GDT to Kernel's GDT

### Init IDT
- Init 8159A chip
- Init IDT items

### Init TSS in GDT

### Init LDT descriptors in GDT

### Init Process Table (includes PCB items)

### Task Switching
The TSS is primarily suited for hardware multitasking, where each individual process has its own TSS. In Software multitasking, one or two TSS's are also generally enough, as they allow for entering Ring 0 code after an interrupt.

There are a lot of subtle things with user mode and task switching that you may not realize at first. First: Whenever a system call interrupt happens, the first thing that happens is the CPU changes to ESP0 stack. Then, it will push all the system information. So when you enter the interrupt handler, your working off of the ESP0 stack. This could become a problem with 2 ring 3 tasks going if all you do is merely push context info and change esp. Think about it. you will change the esp, which is the esp0 stack, to the other tasks esp, which is the same esp0 stack. So, what you must do is change the ESP0 stack(along with the interrupt pushed ESP stack) on each task switch, or you'll end up overwriting yourself.

Whenever a system call occurs, the CPU gets the SS0 and ESP0-value in its TSS and assigns the stack-pointer to it. So one or more kernel-stacks need to be set up for processes doing system calls. Be aware that a thread's/process' time-slice may end during a system call, passing control to another thread/process which may as well perform a system call, ending up in the same stack. Solutions are to *create a private kernel-stack for each thread/process* and re-assign esp0 at any task-switch or to disable scheduling during a system-call.

### Restart: Starting User Processes

## Exceptions and Interrupts Handling

## Locking

## Memory Management
### Physical Memory Management

### Virtual Memory Management

# Process Management
# Task Switch
# Process Schedule
# Process Structure




# File System
## File System Layers


## Superblock


## Meta Data

## Data Storage


## Memory Buffer of File System

## Read and Write
