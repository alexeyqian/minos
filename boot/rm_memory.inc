%ifndef _RM_MEMORY_INC_
%define _RM_MEMORY_INC_

bits 16

; get memory size fo >64M configurations
; ret: ax = KB between 1M and 16M
; ret: bx = number of 64K blocks above 16M
; ret: bx = 0 and ax = -1 error
bios_get_memory_size:
    push ecx
    push edx
    xor ecx, ecx
    xor edx, edx
    mov ax, 0xe801
    int 0x15
    jc .error
    cmp ah, 0x86
    je .error
    cmp ah, 0x80
    je .error
    jcxz .use_ax
    mov ax, cx
    mov bx, dx
.use_ax:
    pop edx
    pop ecx
    ret
.error:
    mov ax, -1
    mov bx, 0
    pop edx
    pop ecx
    ret


; memory map entry structure
struc memory_map_entry
    .base_address resq 1
    .length       resq 1
    .type         resd 1
    .acpi_null    resd 1
endstruc

; get memory map from bios
; in: es:di -. destination buffer for entries
; ret: bp = entry count

bios_get_memory_map:
    pushad
    xor ebx, ebx
    xor bp, bp
    mov edx, 'PAMS'
    mov eax, 0xe820
    mov ecx, 24
    
    int 0x15
    jc .error
    cmp eax, 'PAMS'
    jne .error
    test ebx, ebx
    je  .error
    jmp .start
.next_entry:
    mov edx, 'PAMS'
    mov ecx, 24
    mov eax, 0xe820
    int 0x15
.start:
    jcxz .skip_entry
.notext:
    ecx, [es:di + memory_map_entry.length]
    test ecx, ecx
    jne short .good_entry
    mov ecx, [es:di + memory_map_entry.length + 4]
    jecxz .skip_entry
.good_entry:
    inc bp
    add di, 24
.skip_entry:
    cmp ebx, 0
    jne .next_entry
    jmp .done
.error:
    stc
.done:
    popad
    ret

%endif