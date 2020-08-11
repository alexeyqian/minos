#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "assert.h"
#include "string.h"
#include "kio.h"
#include "ipc.h"
#include "klib.h"

PUBLIC void init_mm(){
    struct boot_params bp;
    get_boot_params(&bp);
    //g_memory_size = bp.mem_size;
    //printl("mm memory size: %dMB\n", g_memory_size / (1024*1024));
}

PUBLIC int alloc_mem(int pid, int memsize){
    assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));
    if(memsize > PROC_IMAGE_SIZE_DEFAULT)
        panic("unsupported memory request: %d, should be less than %d", memsize, PROC_IMAGE_SIZE_DEFAULT);

    int base = PROCS_BASE + (pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;
    if(base + memsize >= g_memory_size)
        panic("memory allocation failed, pid: %d", pid);

    return base;
}

/*
 *  In current version of memory management, the memory block is corresponding with a pid.
 *  So we don't need to really free anything.
 * */
PUBLIC int free_mem(int pid){
    return 0;
}

PUBLIC void task_mm(){
    init_mm();

    while(1){
        send_recv(RECEIVE, ANY, &mm_msg);
        int src = mm_msg.source;
        int reply = 1;

        int msgtype = mm_msg.type;

        switch(msgtype){
            case FORK:
                mm_msg.RETVAL = do_fork();
                break;
            case EXIT:
                do_exit(mm_msg.STATUS);
                reply = 0;
                break;
            case WAIT:
                do_wait();
                reply = 0;
                break;
            default:
                dump_msg("mm: unknow message", &mm_msg);
                assert(0);
                break;
        }

        if(reply){
            mm_msg.type = SYSCALL_RET;
            send_recv(SEND, src, &mm_msg);
        }
    }
}

PUBLIC int fork(){
    MESSAGE msg;
    msg.type = FORK;
    send_recv(BOTH, TASK_MM, &msg);
    assert(msg.type == SYSCALL_RET);
    assert(msg.RETVAL == 0);

    return msg.PID;
}

/*
 * @return 0 if success, otherwise -1 * 
 * */
PUBLIC int do_fork(){
    // find a free slot in proc table
    struct proc* p = proc_table;
    int i;
    for(i = 0; i < NR_TASKS + NR_PROCS; i++, p++){
        if(p->p_flags == FREE_SLOT) break;
    }

    int child_pid = i;
    assert(p == &proc_table[child_pid]);
    assert(child_pid >= NR_TASKS + NR_NATIVE_PROCS);
    if(i >= NR_TASKS + NR_PROCS) return -1;

    // duplicate the process table
    
    int pid = mm_msg.source;
    uint16_t child_ldt_sel = p->ldt_sel;
    *p = proc_table[pid]; // TODO: ?? child_pid
    p->ldt_sel = child_ldt_sel;
    p->p_parent = pid;
    sprintf(p->p_name, "%s_%d", proc_table[pid].p_name, child_pid);

    // duplicate the process: TDS
    struct descriptor* ppd;
    ppd = &proc_table[pid].ldt[INDEX_LDT_C];
    int caller_t_base = reassembly(ppd->base_high, 24, ppd->base_mid, 16, ppd->base_low);
    // depending on the G bit of descriptor, in 1 or 4096 bytes
    int caller_t_limit = reassembly(0, 0, (ppd->limit_high_attr2 & 0xF), 16, ppd->limit_low);
    int caller_t_size = ((caller_t_limit + 1)* ((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8))? 4096 : 1));

    // data & stack segments
    ppd = &proc_table[pid].ldt[INDEX_LDT_RW];
    int caller_ds_base = reassembly(ppd->base_high, 24, ppd->base_mid, 16, ppd->base_low);
    int caller_ds_limit = reassembly((ppd->limit_high_attr2 & 0xf), 16, 0, 0, ppd->limit_low);
    // TODO: ?? caller_t_limit or caller_ds_limit
    int caller_ds_size = ((caller_t_limit+1) * ((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8))? 4096 : 1));

    // we don't seperate t, d and s segments, so we have:
    assert((caller_t_base == caller_ds_base) 
        && (caller_t_limit == caller_ds_limit)
        && (caller_t_size == caller_ds_size));

    int child_base = alloc_mem(child_pid, caller_t_size);
    printl("{mm} 0x%x <- 0x%x (0x%x bytes)\n", child_base, caller_t_base, caller_t_size);
    // child is a copy of parent
    phys_copy((void*)child_base, (void*)caller_t_base, caller_t_size);

    init_descriptor(&p->ldt[INDEX_LDT_C], 
        child_base, 
        (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT, 
        DA_LIMIT_4K | DA_32 | DA_C | PRIVILEGE_USER << 5);

    init_descriptor(&p->ldt[INDEX_LDT_RW],
        child_base,
        (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
        DA_LIMIT_4K | DA_32 | DA_DRW | PRIVILEGE_USER << 5);

    MESSAGE msg2fs;
    msg2fs.type = FORK;
    msg2fs.PID = child_pid;
    send_recv(BOTH, TASK_FS, &msg2fs);

    // child pid will be returned to the parent proc
    mm_msg.PID = child_pid;

    // birth of child 
    MESSAGE m;
    m.type = SYSCALL_RET;
    m.RETVAL = 0;
    m.PID = 0;
    send_recv(SEND, child_pid, &m);

    return 0;
}


/**
 * Do the last jobs to clean up a proc thoroughly
 * send proc's parent a message to unblock it, and
 * release proc's proc_table[] entry * 
 * */
PRIVATE void cleanup(struct proc* proc){
    MESSAGE msg2parent;
    msg2parent.type = SYSCALL_RET;
    msg2parent.PID = proc2pid(proc);
    msg2parent.STATUS = proc->exit_status;
    send_recv(SEND, proc->p_parent, &msg2parent);

    proc->p_flags = FREE_SLOT;
    printl("{mm} cleanup() %s(%d) has been cleand up.\n", proc->p_name, proc2pid(proc));
}


/*
 * Perform the exit() syscall.
 * If proc A calls exit(), then mm will do the following in this routine:
 * 1. inform fs so that the fd-related things will be cleaned up.
 * 2. tell task_sys
 * 3. free A's memory
 * 4. set A.exit_status, which is for the parent
 * 5. depends on parent's status. if parent (say P) is:
 *    (1) WAITING
 *        - clean P's WAITNG bit, and send P a message to unblock it.
 *        - release A's proc_table[] slot
 *    (2) not WAITING
 *        - set A's HANGING bit
 * 6.iterate proc_table[], if proc B is found as A's child, then:
*           (1) make INIT the new parent of B, and
*           (2) if INIT is WAITING and B is HANGING, then:
*                 - clean INIT's WAITING bit, and
*                 - send INIT a message to unblock it
*                   {INIT's wait() call is done}
*                 - release B's proc_table[] slot
*                   {B's exit() call is done}
*               else
*                 if INIT is WAITING but B is not HANGING, then
*                     - B will call exit() and things will be done at
*                       do_exit()::comment::<5>::(1)
*                 if B is HANGING but INIT is not WAITING, then
*                     - INIT will call wait() and things will be doen at
*                       do_wait()::comment::<1>
 * 
 * */
PUBLIC void do_exit(int status){
    int i;
    int pid = mm_msg.source; // pid of caller
    int parent_pid = proc_table[pid].p_parent;
    struct proc* p = &proc_table[pid];

    MESSAGE msg2fs;
    msg2fs.type = EXIT;
    msg2fs.PID = pid;
    send_recv(BOTH, TASK_FS, &msg2fs);

    /**
	 * @todo should also send a message to TASK_SYS to do some cleanup work.
	 *       e.g. if the proc is killed by another proc, TASK_SYS should
	 *            check if the proc happens to be SENDING a message, if so,
	 *            the proc should be removed from the sending queue.
	 * @see MINIX::src/kernel/system.c:do_xit()
	 */

    free_mem(pid);

    p->exit_status = status;

    if(proc_table[parent_pid].p_flags & WAITING){ // parent is waiting
        printl("{MM} ((--do_exit():: %s (%d) is WAITING, %s (%d) will be cleaned up.--))\n",
		       proc_table[parent_pid].p_name, parent_pid,
		       p->p_name, pid);
		printl("{MM} ((--do_exit():1: proc_table[parent_pid].p_flags: 0x%x--))\n",
		       proc_table[parent_pid].p_flags);
        proc_table[parent_pid].p_flags &= ~WAITING;
        cleanup(&proc_table[pid]);
    }else{ // parent is not waiting
        printl("{MM} ((--do_exit():: %s (%d) is not WAITING, %s (%d) will be HANGING--))\n",
		       proc_table[parent_pid].p_name, parent_pid,
		       p->p_name, pid);
        proc_table[pid].p_flags |= HANGING;
    }

    // if the proc has any child, make INIT the new parent
    for(i = 0; i < NR_TASKS + NR_PROCS; i++){
        if(proc_table[i].p_parent == pid){
            proc_table[i].p_parent == INIT; // FIXME: make sure init always waiting
            printl("{MM} %s (%d) exit(), so %s (%d) is INIT's child now\n",
			       p->p_name, pid, proc_table[i].p_name, i);
			printl("{MM} ((--do_exit():2: proc_table[INIT].p_flags: 0x%x--))\n",
			       proc_table[INIT].p_flags);

            if((proc_table[INIT].p_flags & WAITING) &&
                (proc_table[i].p_flags & HANGING)){
                proc_table[INIT].p_flags &= ~WAITING;
                cleanup(&proc_table[i]);
                assert(0);
            }else {assert(0);}           

        }
    }
}

/**
 * Perform the wait() syscall.
 *
 * If proc P calls wait(), then MM will do the following in this routine:
 *     <1> iterate proc_table[],
 *         if proc A is found as P's child and it is HANGING
 *           - reply to P (cleanup() will send P a messageto unblock it)
 *             {P's wait() call is done}
 *           - release A's proc_table[] entry
 *             {A's exit() call is done}
 *           - return (MM will go on with the next message loop)
 *     <2> if no child of P is HANGING
 *           - set P's WAITING bit
 *             {things will be done at do_exit()::comment::<5>::(1)}
 *     <3> if P has no child at all
 *           - reply to P with error
 *             {P's wait() call is done}
 *     <4> return (MM will go on with the next message loop)
 **/
PUBLIC void do_wait(){
    printl("{mm} ((--do_wait()--))");
    int pid = mm_msg.source;
    int i;
    int children = 0;
    struct proc* p_proc = proc_table;

    for(i = 0; i < NR_TASKS + NR_PROCS; i++, p_proc++){
        if(p_proc->p_parent == pid){
            children++;
            if(p_proc->p_flags & HANGING){
                cleanup(p_proc);
                return; // TODO: ??
            }
        }
    }

    if(children){
        proc_table[pid].p_flags |= WAITING;
    }else{
        MESSAGE msg;
        msg.type =  SYSCALL_RET;
        msg.PID = NO_TASK;
        send_recv(SEND, pid, &msg);
    }
}