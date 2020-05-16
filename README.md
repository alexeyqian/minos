# minos
Minimal OS Kernel

The best way to study OS is to make a simple one by yourself.

# Boot Loader
## Boot
It's the boot sector on block on floppy disk/hard disk, which is the first block contains 512 bytes.
It includes the 0xaa55 at locataion 0x510 and 0x511, the magic number which makes BIOS think it's bootable.
It also includes some functions for furthur booting and loading.
### Read Data from Disk Sectors

### Parse FAT12 File System

### Load the Loader

## Loader

### Load Kernel Image into Memory from Disk 

### Enter Protected Mode

#### Setup GDT and IDT

### Parsing ELF File Format

### Load Kernel into Memory

### Transfer Control to Kernel


# Kernel

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
