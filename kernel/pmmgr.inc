; physical memory manager

; The purpose of a physical memory manager is to split up the computers physical address 
; space into block-sized chunks of memory and provide a method to allocate and release them.
; During initialization, the BIOS abtains memory information 
; from the memory controller and configures the chipset to work with the detected memory. 
; all PCs are required to have a memory region just below the 4 GB mark for 
; use by additional possible devices (Memory mapped hardware or the BIOS ROM)



