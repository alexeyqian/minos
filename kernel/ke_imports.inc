
; extern global variable
extern gdt_ptr
extern idt_ptr
extern tss
extern p_proc_ready
extern k_reenter ; reenter for the same interrupt
extern irq_table
extern syscall_table
; extern functions
extern kinit
extern exception_handler
; extern irq_handler
;extern clock_handler