#include "thr_mailbox.h"
#include "thrtab.h"
#include "message.h"
#include "io_message.h"
#include "refarray.h"
#include "predicate.h"
#include "message_predicates.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "mailbox_defs.h"

#if DO_DEBUG_LIBPDTHREAD_THR_MAILBOX == 1
#define DO_DEBUG_LIBPDTHREAD 1
#else
#define DO_DEBUG_LIBPDTHREAD 0
#endif

#include "thr_debug.h"

#undef DO_DEBUG_LIBPDTHREAD

struct fd_set_populator {
    fd_set* to_populate;
    dllist<PDDESC,dummy_sync>* already_ready;
    
    int count;

    void reset() {
        FD_ZERO(to_populate);
        count=0;
    }

    /* FIXME: using fd_set, not portable to non-unix */
    void visit(PDDESC desc) {
        if(!already_ready->contains(desc)) {
            FD_SET(desc, to_populate);
            count++;
        }
    }
};

struct ready_fds_populator {
    fd_set* populate_from;
    thr_mailbox* owner;

    dllist<PDDESC,dummy_sync>* descs_from;    
    dllist<PDDESC,dummy_sync>* populate_to;

    void visit(PDDESC desc) {
        io_entity *ie = file_q::file_from_desc(desc);
        
        if (!(populate_to->contains(desc)) && descs_from->contains(desc)) {
            if (!(ie->will_block()) 
                || FD_ISSET(desc, populate_from)) {
                message *m = new io_message(ie, ie->self(), MSG_TAG_FILE);
                owner->put(m);
                populate_to->put(desc);
            }
        }
    }
};

void* thr_mailbox::fds_select_loop(void* which_mailbox) {
    thr_mailbox* owner = (thr_mailbox*)which_mailbox;

    assert(thr_type(owner->owned_by) == item_t_thread);

    thr_debug_msg(CURRENT_FUNCTION, "entering fds_select_loop for mailbox owned by %d\n", owner->owned_by);

    fd_set* readable_fds = new fd_set;
    fd_set_populator fd_visitor;
    ready_fds_populator ready_visitor;
    
    pthread_sync* fds_monitor = owner->fds_monitor;
    fd_visitor.to_populate = readable_fds;

    while(1) {
        thr_debug_msg(CURRENT_FUNCTION, "LOOPING in fds_select_loop for mailbox owned by %d\n", owner->owned_by);
        int select_status;
        char buf[256];
        
        fd_visitor.already_ready = owner->ready_fds;
        
        if(owner->kill_fd_selector) goto done;

        fd_visitor.reset();
        FD_SET(owner->fds_selector_pipe[0],fd_visitor.to_populate);

        fds_monitor->lock();
        owner->bound_fds->visit(&fd_visitor);
        fds_monitor->unlock();

        select_status = 
            select(fd_visitor.count,fd_visitor.to_populate,NULL,NULL,NULL);

        if(FD_ISSET(owner->fds_selector_pipe[0],fd_visitor.to_populate))
            read(owner->fds_selector_pipe[0],buf,256);
        
        if(select_status == -1) {
            perror("select thread");
        } else if (select_status == 1 && 
                   FD_ISSET(owner->fds_selector_pipe[0],
                            fd_visitor.to_populate)) {
            if(owner->kill_fd_selector) goto done;

            // we've been signaled to add some additional fd's to the
            // wait set, or to check will_block status of bound fds; 
            // we'll check will_block status and loop around

            fds_monitor->lock();            
            
            ready_visitor.owner = owner;
            
            ready_visitor.descs_from = owner->bound_fds;
            ready_visitor.populate_to = owner->ready_fds;       

            owner->ready_fds->visit(&ready_visitor);
            
            fds_monitor->unlock();

            continue;
        } else if(fd_visitor.to_populate != NULL) {
            // since we're waking up anyway, we don't need to worry
            // about the selector_pipe waking us up 
            FD_CLR(owner->fds_selector_pipe[0],fd_visitor.to_populate);
            if(owner->kill_fd_selector) goto done;

            fds_monitor->lock();            
            
            ready_visitor.owner = owner;

            ready_visitor.descs_from = owner->bound_fds;
            ready_visitor.populate_to = owner->ready_fds;            

            owner->ready_fds->visit(&ready_visitor);
            
            fds_monitor->unlock();

            // FIXME:  we're not waiting on this; dike it out?
            fds_monitor->signal(FILE_AVAIL);
            owner->monitor->signal(RECV_AVAIL);
        } else {
            // spurious wakeup; re-select
            continue;
        }
    }
    
  done:
    close(owner->fds_selector_pipe[0]);
    close(owner->fds_selector_pipe[1]);
    
    delete readable_fds;
    return 0;
}

struct sock_set_populator {
    fd_set* to_populate;
    dllist<PDSOCKET,dummy_sync>* already_ready;
    
    int count;

    void reset(PDSOCKET selector_fd) {
        FD_ZERO(to_populate);
        FD_SET(selector_fd, to_populate);
        count=selector_fd + 1;
    }
    
    /* FIXME: using fd_set, not portable to non-unix */
    void visit(PDSOCKET desc);
};


void sock_set_populator::visit(PDSOCKET desc) {
	if(!already_ready->contains(desc)) {
		thr_debug_msg(CURRENT_FUNCTION, "ADDING %d to wait set\n", (unsigned)desc);
		FD_SET(desc, to_populate);
		if(desc + 1 > count) count = desc + 1;
	} else {
		thr_debug_msg(CURRENT_FUNCTION, "NOT ADDING %d to wait set\n", (unsigned)desc);
	}
}



struct ready_socks_populator {
    fd_set* populate_from;
    thr_mailbox* owner;

    dllist<PDSOCKET,dummy_sync>* descs_from;    
    dllist<PDSOCKET,dummy_sync>* populate_to;

    void visit(PDSOCKET desc);
	
};


void
ready_socks_populator::visit(PDSOCKET desc)
{
	io_entity *ie = socket_q::socket_from_desc(desc);

	assert(ie);

#if READY
	if(!(populate_to->contains(desc)) && descs_from->contains(desc)) {
#endif // READY
		if (FD_ISSET(desc, populate_from)) {
			
			thr_debug_msg(CURRENT_FUNCTION, "enqueuing SOCKET message from %d\n", (unsigned)desc);

			message *m = new io_message(ie,ie->self(), MSG_TAG_SOCKET);
			
			owner->put_sock(m);
			populate_to->put(desc);
		} 
#if READY
		else {
			thr_debug_msg(CURRENT_FUNCTION, "NOT enqueuing SOCKET message from %d\n", (unsigned)desc);
		}
	}
#endif // READY
}
    



void* thr_mailbox::sock_select_loop(void* which_mailbox) {
    thr_mailbox* owner = (thr_mailbox*)which_mailbox;

    assert(thr_type(owner->owned_by) == item_t_thread);

    thr_debug_msg(CURRENT_FUNCTION, "ENTERING for mailbox owned by %d\n", owner->owned_by);

    fd_set* readable_socks = new fd_set;
    sock_set_populator sock_visitor;
    ready_socks_populator ready_visitor;
    
    pthread_sync* monitor = owner->sock_monitor;
    sock_visitor.to_populate = readable_socks;

    while(1) {
        thr_debug_msg(CURRENT_FUNCTION, "LOOPING for mailbox owned by %d\n", owner->owned_by);
        int select_status;
        char buf[256];

        sock_visitor.already_ready = owner->ready_socks;
        
        if(owner->kill_sock_selector) goto done;

        sock_visitor.reset(owner->sock_selector_pipe[0]);

        monitor->lock();
        owner->bound_socks->visit(&sock_visitor);
        monitor->unlock();


        thr_debug_msg(CURRENT_FUNCTION, "SELECTING for mailbox owned by %d; count = %d\n", owner->owned_by, sock_visitor.count);
        select_status = 
            select(sock_visitor.count,sock_visitor.to_populate,NULL,NULL,NULL);
        thr_debug_msg(CURRENT_FUNCTION, "SELECT RETURNING for mailbox owned by %d; status = %d\n", owner->owned_by, select_status);

        if(FD_ISSET(owner->sock_selector_pipe[0],sock_visitor.to_populate))
            read(owner->sock_selector_pipe[0],buf,256);

        if(select_status == -1) {
            perror("select thread");
        } else if(select_status) {
            // are we waking up because the controller thread wants us to?
            FD_CLR(owner->sock_selector_pipe[0],sock_visitor.to_populate);
            if(owner->kill_sock_selector) goto done;

            // get ready to deliver some messages....
            monitor->lock();
            
            ready_visitor.owner = owner;

            ready_visitor.populate_from = sock_visitor.to_populate;
            
            ready_visitor.descs_from = owner->bound_socks;
            ready_visitor.populate_to = owner->ready_socks;            

            owner->bound_socks->visit(&ready_visitor);
            monitor->unlock();

            // we don't need to signal that a recv is available
            // because the put() operation in the visitor does it for us
            
        } else {
            //spurious wakeup; lather, rinse, repeat
            continue;
        }
    }
    
  done:
    close(owner->sock_selector_pipe[0]);
    close(owner->sock_selector_pipe[1]);
    
    delete readable_socks;
    return 0;
}

thr_mailbox::thr_mailbox(thread_t owner) 
    : mailbox(owner) {
    thr_debug_msg(CURRENT_FUNCTION, "building mailbox for %d\n", owner);
 
    assert(owned_by == owner);

    messages = new dllist<message*,dummy_sync>;
    sock_messages = new dllist<message*,dummy_sync>;
    
    ready_fds = new dllist<PDDESC,dummy_sync>;
    bound_fds = new dllist<PDDESC,dummy_sync>;

    ready_socks = new dllist<PDSOCKET,dummy_sync>;
    bound_socks = new dllist<PDSOCKET,dummy_sync>;

    fds_monitor = new pthread_sync();
    fds_monitor->register_cond(FILE_AVAIL);

    sock_monitor = new pthread_sync();
    sock_monitor->register_cond(SOCK_AVAIL);

    kill_fd_selector = 0;
    kill_sock_selector = 0;

    fds_selector_pipe = new int[2];
    sock_selector_pipe = new int[2];

    int pipe_success = pipe(fds_selector_pipe);
    assert(pipe_success == 0);    

    pipe_success = pipe(sock_selector_pipe);
    assert(pipe_success == 0);

    // create sock and fd selector threads
    pthread_create(&fd_selector_thread, NULL, 
                   thr_mailbox::fds_select_loop, this);
    pthread_create(&sock_selector_thread, NULL, 
                   thr_mailbox::sock_select_loop, this);    
}

thr_mailbox::~thr_mailbox() {
    delete messages;
    delete ready_fds;
    delete bound_fds;

    delete ready_socks;
    delete bound_socks;

    kill_fd_selector = 0;
    kill_sock_selector = 0;

    signal_fd_selector();
    signal_sock_selector();

    pthread_join(fd_selector_thread, NULL);
    pthread_join(sock_selector_thread, NULL);

    close(fds_selector_pipe[0]);
    close(fds_selector_pipe[1]);

    close(sock_selector_pipe[0]);
    close(sock_selector_pipe[1]);

    delete [] fds_selector_pipe;
    delete [] sock_selector_pipe;
}

inline bool thr_mailbox::check_for(thread_t* sender, tag_t* type, bool do_block, 
                               bool do_yank, message** m, unsigned io_first) {
    bool found = false;
    thread_t actual_sender = 0;
    tag_t actual_type = 0;

    message* msg_from_special = NULL;
    match_message_pred criterion(*sender,*type);
    dllist<message*,dummy_sync> *yank_from = NULL;
    
  find_msg:
    actual_sender = 0;
    actual_type = 0;

    if(io_first)
        found = (messages->contains(&criterion) && (yank_from = messages)) ||
            (sock_messages->contains(&criterion) && (yank_from = sock_messages));
    else 
        found = (sock_messages->contains(&criterion) && (yank_from = sock_messages))
            || (messages->contains(&criterion) && (yank_from = messages));
    
    actual_sender = criterion.actual_sender;
    actual_type = criterion.actual_type;

    // FIXME:  re-write this so that it does a multi-pass check;
    // checking for message types in the proper order (i.e. 
    // file,socket,thread if io_first, otherwise thread,file,socket)
    
    if(found) {
        if(do_yank && m) {
            *m = yank_from->yank(&criterion);
        } else if( !do_yank &&
                 thrtab::is_io_entity(actual_sender) &&
                 ((io_entity*)thrtab::get_entry(actual_sender))->is_special()) {
            // consume the "message" from our special sender;
            // we're telling the caller about input on
            // their special file, and we expect them to
            // consume it
            msg_from_special = yank_from->yank(&criterion);
            m = &msg_from_special;
        }

        goto done;
    } else if(!found && do_block) {
        monitor->wait(RECV_AVAIL);
        goto find_msg;
    }
    
  done:
    
    if(m && *m) {
        if((*m)->type() == MSG_TAG_FILE) {
            PDDESC fd = ((file_q*)thrtab::get_entry((*m)->from()))->fd;
            fds_monitor->lock();
            ready_fds->yank(fd);
            fds_monitor->unlock();
            signal_fd_selector();
        }

        if((*m)->type() == MSG_TAG_SOCKET) {
            PDSOCKET sock = ((socket_q*)thrtab::get_entry((*m)->from()))->sock;
            sock_monitor->lock();
            ready_socks->yank(sock);
            sock_monitor->unlock();
            signal_sock_selector();           
        }
    }       

    if(found) {
        if(actual_sender) *sender = actual_sender;
        if(actual_type) *type = actual_type;
    }
    
	if(msg_from_special != NULL) {
		// consume the "message" from the special sender
		delete msg_from_special;
	}

    return found;
}

int thr_mailbox::put(message* m) {
    monitor->lock();

    messages->put(m);
    
    monitor->unlock();
    monitor->signal(RECV_AVAIL);

    return THR_OKAY;
}

int thr_mailbox::put_sock(message* m) {
    monitor->lock();

    sock_messages->put(m);
    
    monitor->unlock();
    monitor->signal(RECV_AVAIL);

    return THR_OKAY;
}

int thr_mailbox::recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp) {
    int retval = THR_OKAY;

#if READY
    signal_fd_selector();
    signal_sock_selector();
#endif // READY

    monitor->lock();
    
    message* to_recv;
    bool did_recv;

    did_recv = check_for(sender, tagp, true, true, &to_recv);
    
    if(!did_recv) {
        retval = THR_ERR;
        goto done;
    }
    
    *sender = to_recv->deliver(tagp, buf, countp);
    
    delete to_recv;

  done:
    monitor->unlock();
    return retval;
}

void thr_mailbox::signal_fd_selector() {
    static char f = '0';
    write(fds_selector_pipe[1], &f, sizeof(char));
}
    
void thr_mailbox::signal_sock_selector() {
    static char s = '0';
    write(sock_selector_pipe[1], &s, sizeof(char));
}    

int thr_mailbox::poll(thread_t* from, tag_t* tagp, unsigned block,
                      unsigned fd_first) { 
    // FIXME: implement poll_preference
    //        by passing fd_first to check_for and then implementing 
    //        poll_preference semantics there
    
#if READY
    signal_fd_selector();
    signal_sock_selector();
#endif // READY
    
    monitor->lock();
    
    bool found = check_for(from, tagp, (block != 0));
    
    monitor->unlock();

    if(found) {
        return THR_OKAY;
    } else {
        // FIXME:  also set TSD errno
        return -1;
    }
}

void thr_mailbox::bind_fd(PDDESC fd, unsigned special, int
                          (*wb)(void*), void* desc, thread_t* ptid) {
    // FIXME:  ignoring "special"
    *ptid = thrtab::create_file(fd,owned_by,wb,desc,(special != 0));
    fds_monitor->lock();
    bound_fds->put(fd);
    fds_monitor->unlock();

    signal_fd_selector();
}

void thr_mailbox::bind_sock(PDSOCKET sock, unsigned special, int
                            (*wb)(void*), void* desc, thread_t* ptid) {
    *ptid = thrtab::create_socket(sock,owned_by,wb,desc,(special != 0));
    sock_monitor->lock();
    bound_socks->put(sock);
    sock_monitor->unlock();

    signal_sock_selector();
}
