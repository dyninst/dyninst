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
#include <sys/poll.h>
#include <unistd.h>
#include "mailbox_defs.h"

#if DO_DEBUG_LIBPDTHREAD_THR_MAILBOX == 1
#define DO_DEBUG_LIBPDTHREAD 1
#else
#define DO_DEBUG_LIBPDTHREAD 0
#endif

#define CURRENT_FILE thr_mailbox_C
#include "thr_debug.h"

#undef DO_DEBUG_LIBPDTHREAD


struct fd_set_populator
{
	thr_mailbox* mbox;
    fd_set* fds;
    int max_fd;				// maximum fd value in the fd_set


	fd_set_populator( thr_mailbox* mbox_, fd_set* fds_ )
	  : mbox( mbox_ ),
		fds( fds_ ),
		max_fd( 0 )
	{ }

    void reset(PDSOCKET wakeup_fd)
	{
        FD_ZERO(fds);
        FD_SET(wakeup_fd, fds);
        max_fd=wakeup_fd + 1;
    }
    
    /* FIXME: using fd_set, not portable to non-unix */
    bool visit(PDSOCKET desc);
};


bool
fd_set_populator::visit(PDSOCKET desc)
{
	if(!mbox->ready_socks->contains(desc))
	{
		thr_debug_msg(CURRENT_FUNCTION, "ADDING %d to wait set\n", (unsigned)desc);

		FD_SET(desc, fds);
		if( desc + 1 > max_fd )
		{
			max_fd = desc + 1;
		}
	}
	else
	{
		thr_debug_msg(CURRENT_FUNCTION, "NOT ADDING %d to wait set\n", (unsigned)desc);
	}

	return true;	// we always want to continue visiting list items
}



struct ready_socks_populator
{
    thr_mailbox* mbox;
    fd_set* fds;

	ready_socks_populator( thr_mailbox* mbox_, fd_set* fdset_ )
	  : mbox( mbox_ ),
		fds( fdset_ )
	{ }

    bool visit(PDSOCKET desc);
};


bool
ready_socks_populator::visit(PDSOCKET desc)
{
	io_entity *ie = socket_q::socket_from_desc(desc);

	assert(ie);

	// bound_socks had better contain desc, because
	// we are visiting every item in the bound socks set
	assert( mbox->bound_socks->contains(desc) );

	if( !mbox->ready_socks->contains(desc) )
	{
		if(FD_ISSET(desc, fds))
		{
			thr_debug_msg(CURRENT_FUNCTION, "enqueuing SOCKET message from %d\n", (unsigned)desc);

			message *m = new io_message(ie,ie->self(), MSG_TAG_SOCKET);
			
			mbox->put_sock(m);
			mbox->ready_socks->put(desc);
		} 

		else {
			thr_debug_msg(CURRENT_FUNCTION, "NOT enqueuing SOCKET message from %d\n", (unsigned)desc);
		}
	}
	return true;	// we always want to continue visiting list items
}
    



void
thr_mailbox::wait_for_input( void )
{
    assert(thr_type(owned_by) == item_t_thread);

    thr_debug_msg(CURRENT_FUNCTION, "ENTERING for mailbox owned by %d\n", owned_by);

	// allocate a set to hold the sockets that we will check
	fd_set* socks_to_check = new fd_set;

	// build a visitor that will populate the set of sockets we will check
	fd_set_populator select_set_populator( this, socks_to_check );
    
	bool done = false;
    while( !done )
	{
        thr_debug_msg(CURRENT_FUNCTION, "LOOPING for mailbox owned by %d\n", owned_by);

        
		// populate the set of sockets we will check for read readiness
		// start with our "message available" pipe...
        select_set_populator.reset( msg_avail_pipe[0] );

		// ...and add the rest of our bound sockets that have not
		// already been indicated as "ready-to-read"
        sock_monitor->lock();
        bound_socks->visit(&select_set_populator);
        sock_monitor->unlock();


        thr_debug_msg(CURRENT_FUNCTION, "SELECTING for mailbox owned by %d; count = %d\n", owned_by, select_set_populator.max_fd);
        int select_status = select( select_set_populator.max_fd,
										select_set_populator.fds,
										NULL,
										NULL,
										NULL);	// block until something ready
        thr_debug_msg(CURRENT_FUNCTION, "SELECT RETURNING for mailbox owned by %d; status = %d\n", owned_by, select_status);

        if(select_status == -1)
		{
            perror("select thread");
        }
		else if( select_status > 0 )
		{
            // check each of our bound sockets to see if they were found
			// to have data available for reading
			// if a socket is ready to read, add it to the "ready-to-read" set
    		ready_socks_populator set_ready_socks_visitor( this,
													select_set_populator.fds );
            sock_monitor->lock();
            bound_socks->visit(&set_ready_socks_visitor);
            sock_monitor->unlock();

			// if we popped out of the select because someone has
			// put a message into our message queue, we have nothing else
			// to do here - our caller will notice that there is a message
			// available in the message queues
			done = true;

        } else {
			// spurious wakeup - we shouldn't have woken up,
			// because we have an infinite timeout
        	thr_debug_msg(CURRENT_FUNCTION, "SELECT spurious wakeup for mailbox owned by %d; status = %d\n", owned_by, select_status);
        }
    }
    
    delete socks_to_check;
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

    int pipe_success = pipe( msg_avail_pipe );
    assert(pipe_success == 0);
}

thr_mailbox::~thr_mailbox() {
    delete messages;

    delete ready_socks;
    delete bound_socks;

    close( msg_avail_pipe[0] );
    close( msg_avail_pipe[1] );
}


struct ready_buffered_visitor
{
	io_entity* ready_ioe;

	ready_buffered_visitor( void )
	  : ready_ioe( NULL )
	{ }

	bool visit( PDSOCKET desc );
};




// TODO this method of checking for ready buffered entities is not necessarily
// fair - we always start visiting from the list's beginning
bool
ready_buffered_visitor::visit( PDSOCKET desc )
{
	bool continueVisiting = true;

	io_entity* ioe = socket_q::socket_from_desc( desc );
	if( (ioe != NULL) && ioe->is_buffer_ready() )
	{
		ready_ioe = ioe;
		continueVisiting = false;
	}
	return continueVisiting;
}


bool
thr_mailbox::is_buffered_special_ready( thread_t* sender, tag_t* type )
{
	bool is_someone_ready = false;

	if( (*type == MSG_TAG_SOCKET) || (*type == MSG_TAG_ANY) )
	{

		if( *sender != THR_TID_UNSPEC )
		{
			// check the given socket to see if it has buffered data available
			if( thrtab::is_io_entity( *sender ) )
			{
				io_entity* ioe = (io_entity*)(thrtab::get_entry( *sender ));
				is_someone_ready = ioe->is_buffer_ready();
			}
		}
		else
		{
			// look for one of our bound socket who has buffered data available
			ready_buffered_visitor rbv;
			bound_socks->visit( &rbv );
			if( rbv.ready_ioe != NULL )
			{
				is_someone_ready = true;
				*sender = rbv.ready_ioe->self();
			}
		}

		if( is_someone_ready )
		{
			*type = MSG_TAG_SOCKET;

		}
	}
	return is_someone_ready;
}



bool thr_mailbox::check_for(thread_t* sender, tag_t* type, bool do_block, 
                               bool do_yank, message** m, unsigned io_first)
{

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

monitor->lock();

	// TODO when is the correct place to clear the message available pipe?
	// do we need to do it on each call?
	clear_msg_avail();

    if(io_first)
	{
		// check if any buffered special socket has input in its buffer
		found = is_buffered_special_ready( sender, type );
		if( found )
		{
			// TODO fix this mess
			criterion.actual_sender = *sender;
			criterion.actual_type = *type;

			// make sure that if our previous call showed we have data
			// available, that we don't mistakenly assume that the
			// data incoming on the underlying socket that was used to
			// fill the buffer is still available
			if( sock_messages->contains(&criterion) )
			{
				yank_from = sock_messages;
			}
		}
		else
		{
			// we have no buffered data, so check whether any
			// of our bound descriptors are ready to read
			found = sock_messages->contains(&criterion);
			if( found )
			{
				yank_from = sock_messages;
			}
			else 
			{
				found = messages->contains(&criterion);
				if( found )
				{
					yank_from = messages;
				}
			}
		}
	}
	else
	{
		found = (messages->contains(&criterion));
		if( found )
		{
			yank_from = messages;
		}
		else
		{
			// check if any buffered special socket still has
			// input to be consumed
			found = is_buffered_special_ready( sender, type );
			if( found )
			{
				// TODO fix this mess
				criterion.actual_sender = *sender;
				criterion.actual_type = *type;

				// make sure that if our previous call showed we have data
				// available, that we don't mistakenly assume that the
				// data incoming on the underlying socket that was used to
				// fill the buffer is still available
				if( sock_messages->contains(&criterion) )
				{
					yank_from = sock_messages;
				}
			}
			else
			{
				// we have no buffered data, so check
				// whether any of our bound descriptors are ready to read
				found = sock_messages->contains(&criterion);
				if( found )
				{
					yank_from = sock_messages;
				}
			}
		}
	}

    monitor->unlock();
    
    actual_sender = criterion.actual_sender;
    actual_type = criterion.actual_type;

    if(found) {

        if(do_yank && m && (yank_from != NULL) ) {

            monitor->lock();
            unsigned oldsize = 
                yank_from->get_size();
            unsigned newsize;

            *m = yank_from->yank(&criterion);

            newsize = 
                yank_from->get_size();
            assert(oldsize == (newsize + 1));

monitor->unlock();

        }

        goto done;
    }
	else if(!found && do_block)
	{
		// we didn't find a message matching the requested criteria,
		// and we were asked to block until we can return
        thr_debug_msg(CURRENT_FUNCTION, "blocking; size of messages = %d, size of sock_messages = %d\n", messages->get_size(), sock_messages->get_size());

		wait_for_input();

        goto find_msg;
    }
    
done:
    

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
    
    unsigned old_size = messages->get_size();
    
    messages->put(m);

    assert(messages->get_size() == old_size + 1);
	raise_msg_avail();
    monitor->unlock();

    return THR_OKAY;
}

int thr_mailbox::put_sock(message* m) {
    monitor->lock();

    unsigned old_size = sock_messages->get_size();

    sock_messages->put(m);

    assert(sock_messages->get_size() == (old_size + 1));
    raise_msg_avail();    
    monitor->unlock();

    return THR_OKAY;
}

int thr_mailbox::recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp) {
    int retval = THR_OKAY;
    
//    monitor->lock();
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
//    monitor->unlock();
    return retval;
}

void
thr_mailbox::raise_msg_avail( void )
{
    static char s = '0';
    write( msg_avail_pipe[1], &s, sizeof(char));
}    

void
thr_mailbox::clear_msg_avail( void )
{
	pollfd pfd;

	pfd.fd = msg_avail_pipe[0];
	pfd.events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND;
	pfd.revents = 0;

	bool done = false;
	while( !done )
	{
		int pollret = ::poll( &pfd, 1, 0 );
		if( pollret == 1 )
		{
			// read a chunk
			static char buf[1024];
			read( msg_avail_pipe[0], buf, 1024 );
		}
		else if( pollret == 0 )
		{
			// we've consumed all the data available on the msg wakeup pipe
			done = true;
		}
		else
		{
			// there was an error
    		thr_debug_msg(CURRENT_FUNCTION, "POLL FAILED: %d\n", errno );
		}
	}
}



int thr_mailbox::poll(thread_t* from, tag_t* tagp, unsigned block,
                      unsigned fd_first)
{ 
//    monitor->lock();
    

    bool found = check_for(from, tagp, (block != 0), false, NULL, fd_first);


 //   monitor->unlock();

    if(found) {
        return THR_OKAY;
    } else {
        // FIXME:  also set TSD errno
        return -1;
    }
}

void thr_mailbox::bind_sock(PDSOCKET sock, unsigned special, int
                            (*wb)(void*), void* desc, thread_t* ptid)
{
    *ptid = thrtab::create_socket(sock,owned_by,wb,desc,(special != 0));
    sock_monitor->lock();
    bound_socks->put(sock);
    sock_monitor->unlock();
}

void thr_mailbox::unbind_sock(PDSOCKET sock)
{
    entity* s = (entity*)(socket_q::socket_from_desc(sock));
    thread_t to_remove = thrtab::get_tid(s);

    sock_monitor->lock();
    bound_socks->yank(sock);    
    sock_monitor->unlock();

    thrtab::remove(to_remove);
}

bool thr_mailbox::is_sock_bound(PDSOCKET sock)
{
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
	if( ready_socks->contains(sock) )
	{
		ready_socks->yank(sock);
	}

	// remove any message from our sock_messages list indicating this
	// socket is ready to read
	// TODO remove either the ready_socks or sock_messages lists
	io_entity* ioe = socket_q::socket_from_desc( sock );
	if( ioe != NULL )
	{
		match_message_pred criterion(ioe->self(), MSG_TAG_SOCKET);
		if( sock_messages->contains(&criterion) )
		{
			message* m = sock_messages->yank(&criterion);
			delete m;
		}	
	}

	sock_monitor->unlock();
}

class message_dumper {
  public:
    int visit(message* m) {
        m->dump("         ");
        return 1;
    }
};

class sock_dumper {
  public:
    int visit(PDSOCKET s) {
        fprintf(stderr, "%d ", s);
        return 1;
    }
};

void thr_mailbox::dump_state() {
    message_dumper md;
    sock_dumper sd;

    if(messages && messages->get_size()) {
        fprintf(stderr, "   number of pending messages: %d\n", messages->get_size());
        fprintf(stderr, "      contents of pending messages:\n");
        messages->visit(&md);
    }
    
    if(sock_messages && sock_messages->get_size()) {
        fprintf(stderr, "   number of pending sock_messages: %d\n", sock_messages->get_size());
        fprintf(stderr, "      contents of pending sock_messages:\n");
        sock_messages->visit(&md);
    }
    
    if(bound_socks && bound_socks->get_size()) {
        fprintf(stderr, "   number of bound sockets: %d\n", bound_socks->get_size());
        fprintf(stderr, "      bound socks are: ");
        bound_socks->visit(&sd);
        fprintf(stderr, "\n");
    }
    
    if(ready_socks && ready_socks->get_size()) {
        fprintf(stderr, "   number of ready sockets: %d\n", ready_socks->get_size());
        fprintf(stderr, "      ready socks are: ");
        ready_socks->visit(&sd);
        fprintf(stderr, "\n");
    }
    
}

