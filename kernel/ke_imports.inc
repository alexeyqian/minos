
; extern global variable
extern gdt_ptr
extern idt_ptr
extern p_proc_ready
extern tss
extern k_reenter ; reenter for the same interrupt
extern irq_table
extern syscall_table
; extern functions
extern kinit
extern kmain
extern exception_handler
extern irq_handler
extern clock_handler
extern kprint
extern delay