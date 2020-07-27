#include "ipc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "ke_asm_utils.h"
#include "syscall.h"
#include "assert.h"
#include "string.h"
#include "klib.h"
#include "kio.h"

#include "clock.h" // using schedule()

// ring 0, this routine is called after p_flags has been set != 0
// it calls schedule to choose another proc as the proc_ready
// this routing doesnot change the p_flags.
PRIVATE void block(struct proc* p){
    assert(p->p_flags);
    schedule();
}

// ring 0, this is a dummy routing. it does nothing actually.
// when it is called, the p_flags should have been cleared (==0)
PRIVATE void unblock(struct proc* p){
    assert(p->p_flags == 0);
}

// ring 0, check where it is safe to send a message from src to dest.
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
				printl("=_=%s", p->p_name);
				do {
					assert(p->p_msg);
					p = proc_table + p->p_sendto;
					printl("->%s", p->p_name);
				} while (p != proc_table + src);
				printl("=_=");

				return 1;
			}
			p = proc_table + p->p_sendto;
		} else break;		
	}
	return 0;
}


// <ring 0> send a message to the dest proc.
// if the dest is blocked waiting for the message, 
// copy the message to it and unlock dest.
// otherwise the caller will be blocked and appended to the dest's sending queue.
// 0 if success
PRIVATE int msg_send(struct proc* current, int dest, MESSAGE* m){
    struct proc* sender = current;
    struct proc* p_dest = proc_table + dest;
    assert(proc2pid(sender) != dest);

    if(deadlock(proc2pid(sender), dest))
        panic(">>> DEADLOCK %s -> %s", sender->p_name, p_dest->p_name);

    if((p_dest->p_flags & RECEIVING) && // dest is waiting for the msg
        (p_dest->p_recvfrom == proc2pid(sender) || p_dest->p_recvfrom == ANY)){
        assert(p_dest->p_msg);
        assert(m);

        phys_copy( va2la(dest, p_dest->p_msg), 
                    va2la(proc2pid(sender), m),
                    sizeof(MESSAGE));
        
        p_dest->p_msg = 0; 
        p_dest->p_flags &= ~RECEIVING; // dest has received the msg
        p_dest->p_recvfrom = NO_TASK;
        unblock(p_dest);

        assert(p_dest->p_flags == 0);
        assert(p_dest->p_msg == 0);
        assert(p_dest->p_recvfrom == NO_TASK);
        assert(p_dest->p_sendto == NO_TASK);
        
        assert(sender->p_flags == 0);
        assert(sender->p_msg == 0);
        assert(sender->p_recvfrom == NO_TASK);
        assert(sender->p_sendto == NO_TASK);
    }else{ // dest is not waiting for the msg
        sender->p_flags |= SENDING;
        assert(sender->p_flags == SENDING);
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

        assert(sender->p_flags == SENDING);
        assert(sender->p_msg != 0);
        assert(sender->p_recvfrom == NO_TASK);
        assert(sender->p_sendto == dest);
    }

    return 0;
}

// <ring 0> try to get a message from the src proc.
// if the src is blocked sending the message, 
// copy the message from it and unlock src.
// Otherwise the caller will be blocked.
PRIVATE int msg_receive(struct proc* current, int src, MESSAGE* m){
    struct proc* p_who_wanna_recv = current;
    struct proc* p_from = 0;
    struct proc* prev = 0;
    int copyok = 0;

    assert(proc2pid(p_who_wanna_recv) != src);

    if((p_who_wanna_recv->has_int_msg) && 
        ((src == ANY) || (src == INTERRUPT))){
        // there is an interrupt needs a p_who_wanna_recv's handling
        // and it is ready to handle it.
        
        MESSAGE msg;
        reset_msg(&msg);
        msg.source = INTERRUPT;
        msg.type = HARD_INT;
        
        assert(m);

        phys_copy(va2la(proc2pid(p_who_wanna_recv), m), (char*)&msg, sizeof(MESSAGE));

        p_who_wanna_recv->has_int_msg = 0;
        assert(p_who_wanna_recv->p_flags == 0);
		assert(p_who_wanna_recv->p_msg == 0);
		assert(p_who_wanna_recv->p_sendto == NO_TASK);
		assert(p_who_wanna_recv->has_int_msg == 0);

        return 0;
    }

    if(src == ANY){
        // pick first proc in the sending queue
        if(p_who_wanna_recv->q_sending){
            p_from = p_who_wanna_recv->q_sending;
            copyok = 1;

            assert(p_who_wanna_recv->p_flags == 0);
			assert(p_who_wanna_recv->p_msg == 0);
			assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
			assert(p_who_wanna_recv->p_sendto == NO_TASK);
			assert(p_who_wanna_recv->q_sending != 0);

			assert(p_from->p_flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->p_recvfrom == NO_TASK);
			assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    }else if(src >= 0 && src < NR_TASKS + NR_PROCS){
        // receive from a certain proc
        p_from = &proc_table[src];
        if((p_from->p_flags & SENDING) &&
            (p_from->p_sendto == proc2pid(p_who_wanna_recv))){
            copyok = 0;

            struct proc* p = p_who_wanna_recv->q_sending;
            assert(p);
            while(p){
                assert(p_from->p_flags & SENDING);
                if(proc2pid(p) == src)
                    break;
                
                prev = p;
                p = p->next_sending;
            }

            assert(p_who_wanna_recv->p_flags == 0);
			assert(p_who_wanna_recv->p_msg == 0);
			assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
			assert(p_who_wanna_recv->p_sendto == NO_TASK);
			assert(p_who_wanna_recv->q_sending != 0);

			assert(p_from->p_flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->p_recvfrom == NO_TASK);
			assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    }

    if(copyok){
        // this porc must have been waiting for this moment in the queue,
        // so we should remove it from the queue.
        if(p_from == p_who_wanna_recv->q_sending){// the 1st one
            assert(prev == 0);
            // receive -> b ->c => receive -> c
            p_who_wanna_recv->q_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }else{
            assert(prev);
            // A->B->C => A->C
            prev->next_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }

        assert(m);
        assert(p_from->p_msg);
        
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

        assert(p_who_wanna_recv->p_flags == RECEIVING);
		assert(p_who_wanna_recv->p_msg != 0);
		assert(p_who_wanna_recv->p_recvfrom != NO_TASK);
		assert(p_who_wanna_recv->p_sendto == NO_TASK);
		assert(p_who_wanna_recv->has_int_msg == 0);
    }

    return 0;
}

void disp_color_str(char* info, int color){
    // TODO: nothing ...
}
// TODO: move
PUBLIC void dump_proc(struct proc* p)
{
	char info[STR_DEFAULT_LEN];
	int i;
	int text_color = MAKE_COLOR(GREEN, RED);

	int dump_len = sizeof(struct proc);

	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_H);
	out_byte(CRTC_DATA_REG, 0);
	out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_L);
	out_byte(CRTC_DATA_REG, 0);

	sprintf(info, "byte dump of proc_table[%d]:\n", p - proc_table); disp_color_str(info, text_color);
	for (i = 0; i < dump_len; i++) {
		sprintf(info, "%x.", ((unsigned char *)p)[i]);
		disp_color_str(info, text_color);
	}

	/* printl("^^"); */

	disp_color_str("\n\n", text_color);
	sprintf(info, "ANY: 0x%x.\n", ANY); disp_color_str(info, text_color);
	sprintf(info, "NO_TASK: 0x%x.\n", NO_TASK); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);

	sprintf(info, "ldt_sel: 0x%x.  ", p->ldt_sel); disp_color_str(info, text_color);
	sprintf(info, "ticks: 0x%x.  ", p->ticks); disp_color_str(info, text_color);
	sprintf(info, "priority: 0x%x.  ", p->priority); disp_color_str(info, text_color);
	sprintf(info, "pid: 0x%x.  ", p->pid); disp_color_str(info, text_color);
	sprintf(info, "name: %s.  ", p->p_name); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	sprintf(info, "p_flags: 0x%x.  ", p->p_flags); disp_color_str(info, text_color);
	sprintf(info, "p_recvfrom: 0x%x.  ", p->p_recvfrom); disp_color_str(info, text_color);
	sprintf(info, "p_sendto: 0x%x.  ", p->p_sendto); disp_color_str(info, text_color);
	sprintf(info, "nr_tty: 0x%x.  ", p->tty_idx); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	sprintf(info, "has_int_msg: 0x%x.  ", p->has_int_msg); disp_color_str(info, text_color);
}

PUBLIC void dump_msg(const char * title, MESSAGE* m)
{
	int packed = 0;
	printl("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)%s}%s",  //, (0x%x, 0x%x, 0x%x)}",
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

// <ring 0>
// sys_sendrec is it's handler/implementation of system call sendrec
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* p_msg, struct proc* p){
	assert(k_reenter == 0); // make sure we are not in ring0
	assert((src_dest >= 0 && src_dest <= NR_TASKS + NR_PROCS) ||
		src_dest == ANY || src_dest == INTERRUPT);

	int ret = 0;
	int caller = proc2pid(p);
	MESSAGE* mla = (MESSAGE*)va2la(caller, p_msg);
	mla->source = caller;

	assert(mla->source != src_dest);

	if(function == SEND){
		ret = msg_send(p, src_dest, p_msg);
		if(ret != 0) return ret;
	}else if(function == RECEIVE){
		ret = msg_receive(p, src_dest, p_msg);
        //printf(">>> ret: %x", ret);
		if(ret != 0) return ret;
	}else{
		panic("{sys_sendrec} invalid function", "%d, send: %d, receive: %d", function, SEND, RECEIVE);
	}

	return 0;
}

// ring 1-3, a wrapper for system call sendrec
// use this, diret call to sendrec should be avoided.
// always return 0;
PUBLIC int send_recv(int function, int src_dest, MESSAGE* p_msg){
    int ret = 0;

    if(function == RECEIVE)
        memset((char*)p_msg, 0, sizeof(MESSAGE));

    switch(function){
        case BOTH:
            ret = sendrec(SEND, src_dest, p_msg);
            if(ret == 0)
                ret = sendrec(RECEIVE, src_dest, p_msg);
            break;
        case SEND:
        case RECEIVE:            
            ret = sendrec(function, src_dest, p_msg);
            break;
        default:
            assert((function == BOTH) || 
                (function == SEND) || (function == RECEIVE));
            break;
    }

    return ret;
}

PUBLIC int get_ticks2(){
    MESSAGE msg;
    reset_msg(&msg);
    msg.type = GET_TICKS2;
    send_recv(BOTH, TASK_SYS, &msg);
    //printf(">>>retval: %x", msg.RETVAL);
    return msg.RETVAL;
}

// ring 1, the main loop of task sys
PUBLIC void task_sys(){
    MESSAGE msg;
    while(1){
        send_recv(RECEIVE , ANY, &msg);
        int src = msg.source;
        switch(msg.type){
            case GET_TICKS2:
                msg.RETVAL = ticks;
                send_recv(SEND, src, &msg);
                break;
            default:
                panic("unknown msg type");
                break;
        }
    }
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
		assert(p->p_flags == 0);
		unblock(p);

		assert(p->p_flags == 0);
		assert(p->p_msg == 0);
		assert(p->p_recvfrom == NO_TASK);
		assert(p->p_sendto == NO_TASK);
	}else
	{
		p->has_int_msg = 1;
	}	
}

