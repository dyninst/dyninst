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

    void visit(PDSOCKET desc);
	
};


void
ready_socks_populator::visit(PDSOCKET desc)
{
	io_entity *ie = socket_q::socket_from_desc(desc);

	assert(ie);

	if( !owner->ready_socks->contains(desc) && descs_from->contains(desc) )
	{
		if (FD_ISSET(desc, populate_from)) {
			
			thr_debug_msg(CURRENT_FUNCTION, "enqueuing SOCKET message from %d\n", (unsigned)desc);

			message *m = new io_message(ie,ie->self(), MSG_TAG_SOCKET);
			
			owner->put_sock(m);
			owner->ready_socks->put(desc);
		} 

		else {
			thr_debug_msg(CURRENT_FUNCTION, "NOT enqueuing SOCKET message from %d\n", (unsigned)desc);
		}
	}

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
    
    ready_socks = new dllist<PDSOCKET,dummy_sync>;
    bound_socks = new dllist<PDSOCKET,dummy_sync>;

    sock_monitor = new pthread_sync();
    sock_monitor->register_cond(SOCK_AVAIL);

    kill_sock_selector = 0;

    sock_selector_pipe = new int[2];

    int pipe_success = pipe(sock_selector_pipe);
    assert(pipe_success == 0);

    // create sock and fd selector threads
    pthread_create(&sock_selector_thread, NULL, 
                   thr_mailbox::sock_select_loop, this);    
}

thr_mailbox::~thr_mailbox() {
    delete messages;

    delete ready_socks;
    delete bound_socks;

    kill_sock_selector = 0;

    signal_sock_selector();

    pthread_join(sock_selector_thread, NULL);

    close(sock_selector_pipe[0]);
    close(sock_selector_pipe[1]);

    delete [] sock_selector_pipe;
}

bool thr_mailbox::check_for(thread_t* sender, tag_t* type, bool do_block, 
                               bool do_yank, message** m, unsigned io_first) {

check_for_again:
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

    if(found) {

        if(do_yank && m) {
            unsigned oldsize = 
                yank_from->get_size();
            unsigned newsize;

            *m = yank_from->yank(&criterion);

            newsize = 
                yank_from->get_size();
            assert(oldsize == (newsize + 1));

        } else if( !do_yank &&
                 thrtab::is_io_entity(actual_sender) &&
                 ((io_entity*)thrtab::get_entry(actual_sender))->is_special()) {
            // consume the "message" from our special sender;
            // we're telling the caller about input on
            // their special file, and we expect them to
            // consume it
            unsigned oldsize = 
                yank_from->get_size();
            unsigned newsize;

            msg_from_special = yank_from->yank(&criterion);
            m = &msg_from_special;

            newsize = 
                yank_from->get_size();
            assert(oldsize == (newsize + 1));

        }

        goto done;
    } else if(!found && do_block) {
        thr_debug_msg(CURRENT_FUNCTION, "blocking; size of messages = %d, size of sock_messages = %d\n", messages->get_size(), sock_messages->get_size());
        monitor->wait(RECV_AVAIL);
        goto find_msg;
    }
    
  done:
    
#if READY
    if(m && *m) {
        if((*m)->type() == MSG_TAG_SOCKET) {
            PDSOCKET sock = ((socket_q*)thrtab::get_entry((*m)->from()))->sock;
            sock_monitor->lock();
            ready_socks->yank(sock);
            sock_monitor->unlock();
            signal_sock_selector();           
        }
    }       
#endif // READY

    if(found) {
        if(actual_sender) *sender = actual_sender;
        if(actual_type) *type = actual_type;
    }
    
	if(msg_from_special != NULL) {
		// consume the "message" from the special sender
		delete msg_from_special;
	}

	// verify that if this is a message saying data is available,
	// that data is actually available
	if( *type == MSG_TAG_SOCKET )
	{
#if READY
		pollfd pfd;
		pfd.fd = thr_socket( *sender );
		pfd.events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;
		pfd.revents = 0;

		int pollret = ::poll( &pfd, 1, 0 );
		if( pollret == 0 )
		{
			// we got a spurious wakeup - go around again
			fprintf( stderr, "tlib: warning: check_for indicated data available on %d, but poll disagrees\n", pfd.fd );

			// do we return false?
			// go around again?
			//   if so, do we clear the ready sock too?
			clear_ready_sock( pfd.fd );
			goto check_for_again;
		}
		else if( pollret == -1 )
		{
			fprintf( stderr, "tlib: warning: check_for poll failed: %d\n", errno );
		}
#else
		fd_set fds;
		PDSOCKET testsock = thr_socket( *sender );
		FD_ZERO( &fds );
		FD_SET( testsock, &fds );
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		int selret = select( testsock + 1, &fds, NULL, NULL, &tv );
		if( selret == 0 )
		{
			// we got a spurious wakeup - go around again
			fprintf( stderr, "tlib: warning: check_for indicated data available on %d, but select disagrees\n", testsock );

			// do we return false?
			// go around again?
			//   if so, do we clear the ready sock too?
			clear_ready_sock( testsock );
			goto check_for_again;
		}
		else if( selret == -1 )
		{
			fprintf( stderr, "tlib: warning: check_for poll failed: %d\n", errno );
		}

#endif // READY
	}

    return found;
}


int thr_mailbox::put(message* m) {
    monitor->lock();
    
    unsigned old_size = messages->get_size();
    
    messages->put(m);

    assert(messages->get_size() == old_size + 1);

    monitor->unlock();
    monitor->signal(RECV_AVAIL);

    return THR_OKAY;
}

int thr_mailbox::put_sock(message* m) {
    monitor->lock();

    unsigned old_size = sock_messages->get_size();

    sock_messages->put(m);

    assert(sock_messages->get_size() == (old_size + 1));
    
    monitor->unlock();
    monitor->signal(RECV_AVAIL);

    return THR_OKAY;
}

int thr_mailbox::recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp) {
    int retval = THR_OKAY;

#if READY
    signal_sock_selector();
#endif // READY

    monitor->lock();
    thr_debug_msg(CURRENT_FUNCTION, "RECEIVING: size of messages = %d, size of sock_messages = %d\n", messages->get_size(), sock_messages->get_size());
    message* to_recv;
    bool did_recv;

    did_recv = check_for(sender, tagp, true, true, &to_recv);

    thr_debug_msg(CURRENT_FUNCTION, "DONE RECEIVING: size of messages = %d, size of sock_messages = %d\n", messages->get_size(), sock_messages->get_size());

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

void thr_mailbox::signal_sock_selector() {
    static char s = '0';
    write(sock_selector_pipe[1], &s, sizeof(char));
}    

int thr_mailbox::poll(thread_t* from, tag_t* tagp, unsigned block,
                      unsigned fd_first) { 
    // FIXME: implement poll_preference
    //        by passing fd_first to check_for and then implementing 
    //        poll_preference semantics there
    
    signal_sock_selector();

    
    monitor->lock();
    
    bool found = check_for(from, tagp, (block != 0), false, NULL, fd_first);

    monitor->unlock();

    if(found) {
        return THR_OKAY;
    } else {
        // FIXME:  also set TSD errno
        return -1;
    }
}

void thr_mailbox::bind_sock(PDSOCKET sock, unsigned special, int
                            (*wb)(void*), void* desc, thread_t* ptid) {
    *ptid = thrtab::create_socket(sock,owned_by,wb,desc,(special != 0));
    sock_monitor->lock();
    bound_socks->put(sock);
    sock_monitor->unlock();

    signal_sock_selector();
}

bool thr_mailbox::is_sock_bound(PDSOCKET sock) {
    
    sock_monitor->lock();
    bool ret = bound_socks->contains(sock);
    sock_monitor->unlock();

    return ret;
}

void clear_ready_sock( PDSOCKET sock )
{
	((thr_mailbox*)lwp::get_mailbox())->clear_ready_sock( sock );
}

void
thr_mailbox::clear_ready_sock( PDSOCKET sock )
{
	sock_monitor->lock();
	assert( ready_socks->contains(sock) );
	ready_socks->yank(sock);
	sock_monitor->unlock();
	signal_sock_selector();           
}


