#include "ipc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "ke_asm_utils.h"
#include "syscall.h"

#include "string.h"
#include "klib.h"
#include "stdio.h"
#include "screen.h"

#include "clock.h" // using schedule()

// <ring 0>, this routine is called after p_flags has been set != 0
// it calls schedule to choose another proc as the proc_ready
// this routing doesnot change the p_flags.
PRIVATE void block(struct proc* p){
    kassert(p->p_flags);
    schedule();
}

// <ring 0>, this is a dummy routing. it does nothing actually.
// when it is called, the p_flags should have been cleared (==0)
PRIVATE void unblock(struct proc* p){
    kassert(p->p_flags == 0);
}

// <ring 0>, check where it is safe to send a message from src to dest.
// the routing will detect if the msaage graph contains a circle.
// for instance, if we have procs trying to send messages like this:
// A->B->C->A, then a deadlock occures, because all of them will wait forever.
// if no cycles detected, it is considered as safe.
// 0 if success
PRIVATE int deadlock(int src, int dest)
{
	struct proc* p = proc_table + dest;
	while (1) {
		if (p->p_flags & SENDING) {
			if (p->p_sendto == src) {
				/* print the chain */
				p = proc_table + dest;
				kprintf("=_=%s", p->p_name);
				do {
					kassert(p->p_msg);
					p = proc_table + p->p_sendto;
					kprintf("->%s", p->p_name);
				} while (p != proc_table + src);
				kprintf("=_=");

				return 1;
			}
			p = proc_table + p->p_sendto;
		} else break;		
	}
	return 0;
}


/*
 * <ring 0> send a message to the dest proc.
 * if the dest is blocked waiting for the message, 
 * copy the message to it and unlock dest.
 * otherwise the caller will be blocked and appended to the dest's sending queue.
 * 
 * @return 0 if success
* */
PRIVATE int msg_send(struct proc* current, int dest, MESSAGE* m){
    struct proc* sender = current;
    struct proc* p_dest = proc_table + dest;
    kassert(proc2pid(sender) != dest);
    
    if(deadlock(proc2pid(sender), dest))
        kpanic(">>> DEADLOCK %s -> %s", sender->p_name, p_dest->p_name);

    if((p_dest->p_flags & RECEIVING) && // dest is waiting for the msg
        (p_dest->p_recvfrom == proc2pid(sender) || p_dest->p_recvfrom == ANY)){
        kassert(p_dest->p_msg);
        kassert(m);

        phys_copy( va2la(dest, p_dest->p_msg), 
                    va2la(proc2pid(sender), m),
                    sizeof(MESSAGE));
        
        p_dest->p_msg = 0; 
        p_dest->p_flags &= ~RECEIVING; // dest has received the msg
        p_dest->p_recvfrom = NO_TASK;
        unblock(p_dest);

        kassert(p_dest->p_flags == 0);
        kassert(p_dest->p_msg == 0);
        kassert(p_dest->p_recvfrom == NO_TASK);
        kassert(p_dest->p_sendto == NO_TASK);
        
        kassert(sender->p_flags == 0);
        kassert(sender->p_msg == 0);
        kassert(sender->p_recvfrom == NO_TASK);
        kassert(sender->p_sendto == NO_TASK);
        //kprintf("message sent: dest: %d, type: %d", dest, m->type);
    }else{ // dest is not waiting for the msg
        sender->p_flags |= SENDING;
        kassert(sender->p_flags == SENDING);
        sender->p_sendto = dest;
        sender->p_msg = m;

        // append to the sending queue
        struct proc* p;
        if(p_dest->q_sending){
            p = p_dest->q_sending;
            while(p->next_sending)
                p = p->next_sending;
            p->next_sending = sender;
        }else
            p_dest->q_sending = sender;
        
        sender->next_sending = 0;
        block(sender);

        kassert(sender->p_flags == SENDING);
        kassert(sender->p_msg != 0);
        kassert(sender->p_recvfrom == NO_TASK);
        kassert(sender->p_sendto == dest);
        //kprintf("sender waiting: dest: %d, type: %d", dest, m->type);
    }

    return 0;
}

/*
 * <ring 0> try to get a message from the src proc.
 * if the src is blocked sending the message, 
 * copy the message from it and unlock src.
 * Otherwise the caller will be blocked.
*/
PRIVATE int msg_receive(struct proc* current, int src, MESSAGE* m){
    struct proc* p_who_wanna_recv = current;
    struct proc* p_from = 0;
    struct proc* prev = 0;
    int copyok = 0;

    kassert(proc2pid(p_who_wanna_recv) != src);

    if((p_who_wanna_recv->has_int_msg) && 
        ((src == ANY) || (src == INTERRUPT))){
        // there is an interrupt needs a p_who_wanna_recv's handling
        // and it is ready to handle it.
        
        MESSAGE msg;
        reset_msg(&msg);
        msg.source = INTERRUPT;
        msg.type = HARD_INT;
        
        kassert(m);

        phys_copy(va2la(proc2pid(p_who_wanna_recv), m), (char*)&msg, sizeof(MESSAGE));

        p_who_wanna_recv->has_int_msg = 0;
        kassert(p_who_wanna_recv->p_flags == 0);
		kassert(p_who_wanna_recv->p_msg == 0);
		kassert(p_who_wanna_recv->p_sendto == NO_TASK);
		kassert(p_who_wanna_recv->has_int_msg == 0);

        return 0;
    }

    if(src == ANY){
        // pick first proc in the sending queue
        if(p_who_wanna_recv->q_sending){
            p_from = p_who_wanna_recv->q_sending;
            copyok = 1;

            kassert(p_who_wanna_recv->p_flags == 0);
			kassert(p_who_wanna_recv->p_msg == 0);
			kassert(p_who_wanna_recv->p_recvfrom == NO_TASK);
			kassert(p_who_wanna_recv->p_sendto == NO_TASK);
			kassert(p_who_wanna_recv->q_sending != 0);

			kassert(p_from->p_flags == SENDING);
			kassert(p_from->p_msg != 0);
			kassert(p_from->p_recvfrom == NO_TASK);
			kassert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    }else if(src >= 0 && src < NR_TASKS + NR_PROCS){
        // receive from a certain proc
        p_from = &proc_table[src];
        if((p_from->p_flags & SENDING) &&
            (p_from->p_sendto == proc2pid(p_who_wanna_recv))){
            copyok = 0;

            struct proc* p = p_who_wanna_recv->q_sending;
            kassert(p);
            while(p){
                kassert(p_from->p_flags & SENDING);
                if(proc2pid(p) == src)
                    break;
                
                prev = p;
                p = p->next_sending;
            }

            kassert(p_who_wanna_recv->p_flags == 0);
			kassert(p_who_wanna_recv->p_msg == 0);
			kassert(p_who_wanna_recv->p_recvfrom == NO_TASK);
			kassert(p_who_wanna_recv->p_sendto == NO_TASK);
			kassert(p_who_wanna_recv->q_sending != 0);

			kassert(p_from->p_flags == SENDING);
			kassert(p_from->p_msg != 0);
			kassert(p_from->p_recvfrom == NO_TASK);
			kassert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    }

    if(copyok){
        // this porc must have been waiting for this moment in the queue,
        // so we should remove it from the queue.
        if(p_from == p_who_wanna_recv->q_sending){// the 1st one
            kassert(prev == 0);
            // receive -> b ->c => receive -> c
            p_who_wanna_recv->q_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }else{
            kassert(prev);
            // A->B->C => A->C
            prev->next_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }

        kassert(m);
        kassert(p_from->p_msg);
        
        phys_copy(va2la(proc2pid(p_who_wanna_recv), m),
            va2la(proc2pid(p_from), p_from->p_msg),
            sizeof(MESSAGE));

        p_from->p_msg = 0;
        p_from->p_sendto = NO_TASK;
        p_from->p_flags &= ~SENDING;

        unblock(p_from);

    }else{ // nobody is sending any msg
        p_who_wanna_recv->p_flags |= RECEIVING;
        p_who_wanna_recv->p_msg = m;
        p_who_wanna_recv->p_recvfrom = src;
        block(p_who_wanna_recv);

        kassert(p_who_wanna_recv->p_flags == RECEIVING);
		kassert(p_who_wanna_recv->p_msg != 0);
		kassert(p_who_wanna_recv->p_recvfrom != NO_TASK);
		kassert(p_who_wanna_recv->p_sendto == NO_TASK);
		kassert(p_who_wanna_recv->has_int_msg == 0);
    }

    return 0;
}

PUBLIC void dump_proc(struct proc* p)
{
	char info[STR_DEFAULT_LEN];
	int i;
	int text_color = MAKE_COLOR(GREEN, RED);

	int dump_len = sizeof(struct proc);

    tty_reset_start_addr();
	
	sprintf(info, "byte dump of proc_table[%d]:\n", p - proc_table); 
    kprintf("%s", info);
	for (i = 0; i < dump_len; i++) {
		sprintf(info, "%x.", ((unsigned char *)p)[i]);
		kprintf("%s", info);
	}

	sprintf(info, "ANY: 0x%x.\n", ANY); kprintf("%s", info);
	sprintf(info, "NO_TASK: 0x%x.\n", NO_TASK); kprintf("%s", info);
	kprintf("%s", info);

	sprintf(info, "ldt_sel: 0x%x.  ", p->ldt_sel); kprintf("%s", info);
	sprintf(info, "ticks: 0x%x.  ", p->ticks); kprintf("%s", info);;
	sprintf(info, "priority: 0x%x.  ", p->priority); kprintf("%s", info);;
	sprintf(info, "name: %s.  \n", p->p_name); kprintf("%s", info);;
	sprintf(info, "p_flags: 0x%x.  ", p->p_flags); kprintf("%s", info);;
	sprintf(info, "p_recvfrom: 0x%x.  ", p->p_recvfrom); kprintf("%s", info);;
	sprintf(info, "p_sendto: 0x%x.  \n", p->p_sendto); kprintf("%s", info);;
	sprintf(info, "has_int_msg: 0x%x.  ", p->has_int_msg); kprintf("%s", info);;
}

PUBLIC void dump_msg(const char * title, MESSAGE* m)
{
	int packed = 0;
	kprintf("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)%s}%s",  //, (0x%x, 0x%x, 0x%x)}",
	       title,
	       (int)m,
	       packed ? "" : "\n        ",
	       proc_table[m->source].p_name,
	       m->source,
	       packed ? " " : "\n        ",
	       m->type,
	       packed ? " " : "\n        ",
	       m->u.m3.m3i1,
	       m->u.m3.m3i2,
	       m->u.m3.m3i3,
	       m->u.m3.m3i4,
	       (int)m->u.m3.m3p1,
	       (int)m->u.m3.m3p2,
	       packed ? "" : "\n",
	       packed ? "" : "\n"/* , */
		);
}

// <ring 0> implementation of system call sendrec
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* pmsg, struct proc* p){
	kassert(k_reenter == 0); // make sure we are not in ring0
	kassert((src_dest >= 0 && src_dest <= NR_TASKS + NR_PROCS) ||
		src_dest == ANY || src_dest == INTERRUPT);
	int ret = 0;
	int caller = proc2pid(p);
	MESSAGE* mla = (MESSAGE*)va2la(caller, pmsg); 
	mla->source = caller;
	kassert(mla->source != src_dest);

	if(function == SEND){
        // TODO: wierd issue: if remove this kprintf, system will halt
        kprintf("%d", src_dest);
        ret = msg_send(p, src_dest, pmsg);
		if(ret != 0) return ret;
	}else if(function == RECEIVE){
		//ret = msg_receive(p, src_dest, mla);
        ret = msg_receive(p, src_dest, pmsg);
		if(ret != 0) return ret;
	}else{
		kpanic("sys_sendrec invalid function", "%d, send: %d, receive: %d", function, SEND, RECEIVE);
	}

	return 0;
}

// <ring 0> inform a proc that an interrupt has occured
PUBLIC void inform_int(int task_nr){
    struct proc* p = proc_table + task_nr;
	if((p->p_flags & RECEIVING) && // dest is wating for the msg
		((p->p_recvfrom == INTERRUPT) || (p->p_recvfrom == ANY))) {
		p->p_msg->source = INTERRUPT;
		p->p_msg->type = HARD_INT;
		p->p_msg = 0; 
		p->has_int_msg = 0; 
		p->p_flags &= ~RECEIVING; 
		p->p_recvfrom = NO_TASK;
		kassert(p->p_flags == 0);
		unblock(p);

		kassert(p->p_flags == 0);
		kassert(p->p_msg == 0);
		kassert(p->p_recvfrom == NO_TASK);
		kassert(p->p_sendto == NO_TASK);
	}else
	{
		p->has_int_msg = 1;
	}	
}

PUBLIC void reset_msg(MESSAGE* p)
{
	memset(p, 0, sizeof(MESSAGE));
}

