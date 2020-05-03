[bits 16]
load_kernel_from_disk:
    mov bx, MSG_LOAD_KERNEL
    call rmprint
    call rmprint_nl

    ; Set-up parameters for our disk_load routine , so 
    ; that we load the first 15 sectors (excluding 
    ; the boot sector) from the boot disk (i.e. our 
    ; kernel code) to address KERNEL_OFFSET

    mov bx, KERNEL_OFFSET
    mov dh, 15                ; Load 15 sectors to ES(0x0000):BX
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret