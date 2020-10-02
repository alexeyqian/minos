
; extern global variable
extern gdt_ptr
extern idt_ptr
extern tss
extern p_proc_ready
extern k_reenter ; reenter for the same interrupt
extern irq_table
extern syscall_table
; extern functions
extern kstart
extern exception_handler