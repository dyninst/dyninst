#include <pthread.h>
#include <errno.h>

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
#include <fcntl.h>
#include "mailbox_defs.h"


#include <strings.h>
#include "common/h/Vector.h"

#if DO_DEBUG_LIBPDTHREAD_THR_MAILBOX == 1
#define DO_DEBUG_LIBPDTHREAD 1
#else
#define DO_DEBUG_LIBPDTHREAD 0
#endif

#define CURRENT_FILE thr_mailbox_C
#include "thr_debug.h"

#undef DO_DEBUG_LIBPDTHREAD

struct FdSetPopulator
{
	thr_mailbox* mbox;
    fd_set* fds;
    int max_fd;				// maximum fd value in the fd_set


	FdSetPopulator( thr_mailbox* mbox_, fd_set* fds_ )
	  : mbox( mbox_ ),
		fds( fds_ ),
		max_fd( 0 )
	{ }

    void reset( PdSocket wakeup_fd )
	{
        FD_ZERO(fds);
        FD_SET(wakeup_fd.s, fds);
        max_fd = wakeup_fd.s + 1;
    }
    
    /* FIXME: using fd_set, not portable to non-unix */
    bool visit( PdSocket desc);
    bool visit( PdFile desc );
};


bool
FdSetPopulator::visit( PdSocket desc)
{
	if(!mbox->ready_socks->contains(desc))
    {
        thr_debug_msg(CURRENT_FUNCTION,
            "ADDING %d to wait set\n", (unsigned)desc.s);
           
        FD_SET(desc.s, fds);
        if( desc.s + 1 > max_fd )
        {
           max_fd = desc.s + 1;
        }
	}
	else
	{
		thr_debug_msg(CURRENT_FUNCTION,
            "NOT ADDING %d to wait set\n", (unsigned)desc.s);
	}

	return true;	// we always want to continue visiting list items
}


bool
FdSetPopulator::visit( PdFile desc)
{
	if(!mbox->ready_files->contains(desc))
    {
        thr_debug_msg(CURRENT_FUNCTION,
            "ADDING %d to wait set\n", (unsigned)desc.fd);
           
        FD_SET(desc.fd, fds);
        if( desc.fd + 1 > max_fd )
        {
           max_fd = desc.fd + 1;
        }
	}
	else
	{
		thr_debug_msg(CURRENT_FUNCTION,
            "NOT ADDING %d to wait set\n", (unsigned)desc.fd);
	}

	return true;	// we always want to continue visiting list items
}




struct ReadySetPopulator
{
    thr_mailbox* mbox;
    fd_set* fds;

	ReadySetPopulator( thr_mailbox* mbox_, fd_set* fdset_ )
	  : mbox( mbox_ ),
		fds( fdset_ )
	{ }

    bool visit( PdSocket desc );
    bool visit( PdFile desc );
};


bool
ReadySetPopulator::visit( PdSocket desc )
{
	io_entity *ie = socket_q::socket_from_desc(desc);

	if(!ie || !mbox->bound_socks->contains(desc))
      return true;

	// bound_socks had better contain desc, because
	// we are visiting every item in the bound socks set
	// assert( mbox->bound_socks->contains(desc) );

	if( !mbox->ready_socks->contains(desc) )
	{
		if(FD_ISSET(desc.s, fds))
		{
			thr_debug_msg(CURRENT_FUNCTION,
                "enqueuing SOCKET message from %d\n", (unsigned)desc.s);

			message *m = new io_message(ie,ie->self(), MSG_TAG_SOCKET);
			
			mbox->put_sock(m);
			mbox->ready_socks->put(desc);
		} 

		else {
			thr_debug_msg(CURRENT_FUNCTION,
                "NOT enqueuing SOCKET message from %d\n", (unsigned)desc.s);
		}
	}
	return true;	// we always want to continue visiting list items
}


bool
ReadySetPopulator::visit( PdFile desc )
{
	io_entity *ie = file_q::file_from_desc(desc);

	if(!ie || !mbox->bound_files->contains(desc))
      return true;

	// bound_files had better contain desc, because
	// we are visiting every item in the bound files set
	// assert( mbox->bound_file->contains(desc) );

	if( !mbox->ready_files->contains(desc) )
	{
		if(FD_ISSET(desc.fd, fds))
		{
			thr_debug_msg(CURRENT_FUNCTION,
                "enqueuing FILE message from %d\n", (unsigned)desc.fd);

			message *m = new io_message(ie,ie->self(), MSG_TAG_FILE);
			
			mbox->put_file(m);
			mbox->ready_files->put(desc);
		} 

		else {
			thr_debug_msg(CURRENT_FUNCTION,
                "NOT enqueuing FILE message from %d\n", (unsigned)desc.fd);
		}
	}
	return true;	// we always want to continue visiting list items
}




// struct with visit operation for finding invalid descriptors
// Intended to be used on the bound_socks and bound_files lists.
// 
struct BadDescFinder
{
    pdvector<PdSocket> socks;
    pdvector<PdFile> fds;

    bool visit(PdSocket desc);
    bool visit(PdFile desc);
};

bool
BadDescFinder::visit( PdSocket desc )
{
    if( (fcntl(desc.s, F_GETFL) == -1) && (errno == EBADF) )
    {
        socks.push_back( desc );
    }
    return true;        // always continue search
}

bool
BadDescFinder::visit( PdFile desc )
{
    if( (fcntl(desc.fd, F_GETFL) == -1) && (errno == EBADF) )
    {
        fds.push_back( desc );
    }
    return true;        // always continue search
}





void
thr_mailbox::wait_for_input( void )
{
    assert(thr_type(owned_by) == item_t_thread);

    thr_debug_msg(CURRENT_FUNCTION, "ENTERING for mailbox owned by %d\n", owned_by);

	// allocate a set to hold the sockets that we will check
	fd_set* fds_to_check = new fd_set;

	// build a visitor that will populate the set of sockets we will check
	FdSetPopulator select_set_populator( this, fds_to_check );
    
	bool done = false;
    while( !done )
	{
        thr_debug_msg(CURRENT_FUNCTION, "LOOPING for mailbox owned by %d\n",
                        owned_by);

        
		// populate the set of sockets we will check for read readiness
		// start with our "message available" pipe...
        select_set_populator.reset( msg_avail_pipe[0] );

		// ...add our bound sockets that have not
		// already been indicated as "ready-to-read"
        sock_monitor->lock();
        bound_socks->visit(&select_set_populator);
        sock_monitor->unlock();

        // ...add our bound files that have not already
        // been indicated as "ready-to-read"
        file_monitor->lock();
        bound_files->visit(&select_set_populator);
        file_monitor->unlock();


        thr_debug_msg(CURRENT_FUNCTION,
                        "SELECTING for mailbox owned by %d; count = %d\n",
                        owned_by, select_set_populator.max_fd);
        int select_status = select( select_set_populator.max_fd,
										select_set_populator.fds,
										NULL,
										NULL,
										NULL);	// block until something ready
        thr_debug_msg(CURRENT_FUNCTION, "SELECT RETURNING for mailbox owned by %d; status = %d\n", owned_by, select_status);

        if(select_status == -1) {
            if( errno == EBADF )
            {
                // we have at least one bad file descriptor in our set of
                // bound descriptors -
                // (sadly, this can happen when a client socket connection is
                // broken without unbinding the bad descriptor first)
                // check them all and unbind the bad descriptor
                // note that we don't unbind them as part of the visit
                // to avoid changing bound_socks from underneath the
                // visit method.
                BadDescFinder bdf;

                // check sockets
                sock_monitor->lock();
                bound_socks->visit( &bdf );

                // unbind the bad descriptors
                for( unsigned int i = 0; i < bdf.socks.size(); i++ )
                {
                    // note we already have the lock
                    unbind( bdf.socks[i], false ); 
                }
                sock_monitor->unlock();

                // check files
                file_monitor->lock();
                bound_files->visit( &bdf );

                // unbind the bad descriptors
                for( unsigned int i = 0; i < bdf.fds.size(); i++ )
                {
                    // note we already have the lock
                    unbind( bdf.fds[i], false ); 
                }
                file_monitor->unlock();
            }
            else
            {
                perror("select thread");
            }
        }
		else if( select_status > 0 )
		{
    		ReadySetPopulator visitor( this, select_set_populator.fds );

            // check each of our bound sockets to see if it has data available
			// if a socket is ready to read, add it to the "ready-to-read" set
            sock_monitor->lock();
            bound_socks->visit(&visitor);
            sock_monitor->unlock();

            // check each of our bound files to see if it has data available
			// if a file is ready to read, add it to the "ready-to-read" set
            file_monitor->lock();
            bound_files->visit(&visitor);
            file_monitor->unlock();

			// if we popped out of the select because someone has
			// put a message into our message queue, we have nothing else
			// to do here - our caller will notice that there is a message
			// available in the message queues
			done = true;

        } else {
			// spurious wakeup - we shouldn't have woken up,
			// because we have an infinite timeout
        	thr_debug_msg(CURRENT_FUNCTION,
                "SELECT spurious wakeup for mailbox owned by %d; status = %d\n",
                owned_by, select_status);
        }
    }
    
    delete fds_to_check;
}

thr_mailbox::thr_mailbox(thread_t owner) 
  : mailbox(owner),
    messages( new dllist<message*> ),
    sock_messages( new dllist<message*> ),
    file_messages( new dllist<message*> ),
    bound_socks( new dllist<PdSocket> ),
    ready_socks( new dllist<PdSocket> ),
    sock_monitor( new pthread_sync("socket monitor for thread mailbox") ),
    bound_files( new dllist<PdFile> ),
    ready_files( new dllist<PdFile> ),
    file_monitor( new pthread_sync("socket monitor for thread mailbox") )
{
    thr_debug_msg(CURRENT_FUNCTION, "building mailbox for %d\n", owner);
 
    assert(owned_by == owner);

    int pipe_success = pipe( msg_avail_pipe );
    assert(pipe_success == 0);

    // Set the write end of the pipe to be non-blocking.
    //
    // A non-blocking pipe is a way to avoid the deadlock that
    // occurs in case the sender sends so many messages that it
    // fills the pipe causing the sender to block writing into the pipe
    // (and holding the receiver's mailbox's mutex), while the receiver
    // is blocked waiting to acquire the mutex in order to consume
    // the bytes from the pipe.
    int fret = fcntl( msg_avail_pipe[1], F_GETFL );
    if( fret >= 0 )
    {
        fret = fcntl( msg_avail_pipe[1], F_SETFL, fret | O_NONBLOCK );
    }
    if( fret < 0 )
    {
        thr_debug_msg(CURRENT_FUNCTION, "unable to set msg_avail pipe to non-blocking mode\n" );
    }
}

thr_mailbox::~thr_mailbox( void )
{
    delete messages;

    delete sock_messages;
    delete ready_socks;
    delete bound_socks;
    delete sock_monitor;

    delete file_messages;
    delete bound_files;
    delete ready_files;
    delete file_monitor;

    close( msg_avail_pipe[0] );
    close( msg_avail_pipe[1] );
}


struct ReadyBufferedVisitor
{
	io_entity* ready_ioe;

	ReadyBufferedVisitor( void )
	  : ready_ioe( NULL )
	{ }

	bool visit( PdSocket desc );
	bool visit( PdFile desc );
};




// TODO this method of checking for ready buffered entities is not necessarily
// fair - we always start visiting from the list's beginning
bool
ReadyBufferedVisitor::visit( PdSocket desc )
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
ReadyBufferedVisitor::visit( PdFile desc )
{
	bool continueVisiting = true;

	io_entity* ioe = file_q::file_from_desc( desc );
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

	if( *type == MSG_TAG_SOCKET )
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
			ReadyBufferedVisitor rbv;
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



bool
thr_mailbox::check_for(thread_t* sender,
                        tag_t* type,
                        bool do_block, 
                        bool do_yank,
                        message** m,
                        unsigned io_first)
{

check_for_again:
    bool found = false;
    thread_t actual_sender = 0;
    tag_t actual_type = 0;

    message* msg_from_special = NULL;
    match_message_pred criterion(*sender,*type);
    dllist<message*> *yank_from = NULL;
    
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
            else if( file_messages->contains(&criterion) )
            {
                yank_from = file_messages;
            }
		}
		else
		{
			// we have no buffered data, so check whether any
			// of our bound descriptors are ready to read
			if( (found = sock_messages->contains(&criterion)) == true )
			{
				yank_from = sock_messages;
			}
			else if( (found = file_messages->contains(&criterion)) == true )
			{
                yank_from = file_messages;
            }
            else if( (found = messages->contains(&criterion)) == true )
            {
                yank_from = messages;
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
				if( (found = sock_messages->contains(&criterion)) == true )
				{
					yank_from = sock_messages;
				}
                else if( (found = file_messages->contains(&criterion)) == true )
                {
                    yank_from = file_messages;
                }
			}
			else
			{
				// we have no buffered data, so check
				// whether any of our bound descriptors are ready to read
				if( (found = sock_messages->contains(&criterion)) == true )
				{
					yank_from = sock_messages;
				}
                else if( (found = file_messages->contains(&criterion)) == true )
                {
                    yank_from = file_messages;
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

            unsigned int oldsize = yank_from->get_size();

            *m = yank_from->yank(&criterion);

            unsigned int newsize = yank_from->get_size();
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

int
thr_mailbox::put_file(message* m)
{
    monitor->lock();

    unsigned old_size = file_messages->get_size();

    file_messages->put(m);

    assert(file_messages->get_size() == (old_size + 1));
    raise_msg_avail();    
    monitor->unlock();

    return THR_OKAY;
}


int thr_mailbox::recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp) {
    int retval = THR_OKAY;
    COLLECT_MEASUREMENT(THR_MSG_TIMER_START);
        
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
    COLLECT_MEASUREMENT(THR_MSG_TIMER_STOP);

    return retval;
}

void
thr_mailbox::raise_msg_avail( void )
{
    static char s = '0';
    ssize_t nWritten = write( msg_avail_pipe[1], &s, sizeof(char));
    if( (nWritten == -1) && (errno == EAGAIN) )
    {
        thr_debug_msg(CURRENT_FUNCTION,
            "write to msg_avail pipe would've blocked\n" );
    }
}    

void
thr_mailbox::clear_msg_avail( void )
{
	pollfd pfd;

	pfd.fd = msg_avail_pipe[0];
	pfd.events = POLLIN;
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
    
    COLLECT_MEASUREMENT(THR_MSG_TIMER_START);

    bool found = check_for(from, tagp, (block != 0), false, NULL, fd_first);


 //   monitor->unlock();

    COLLECT_MEASUREMENT(THR_MSG_TIMER_STOP);

    if(found) {
        return THR_OKAY;
    } else {
        // FIXME:  also set TSD errno
        return -1;
    }
}


void
thr_mailbox::bind( PdSocket sock,
                            unsigned special,
                            int (*wb)(void*), void* arg,
                            thread_t* ptid)
{
    *ptid = thrtab::create_socket(sock,owned_by,wb,arg,(special != 0));
    sock_monitor->lock();
    bound_socks->put(sock);
    sock_monitor->unlock();
}

void
thr_mailbox::bind( PdFile desc,
                        unsigned int special,
                        int (*wb)(void*),
                        void* arg,
                        thread_t* ptid )
{
    *ptid = thrtab::create_file( desc, owned_by, wb, arg, (special != 0) );
    file_monitor->lock();
    bound_files->put( desc );
    file_monitor->unlock();
}

void
thr_mailbox::unbind( PdSocket sock, bool getlock )
{
    entity* s = (entity*)(socket_q::socket_from_desc(sock));
    if (s) {
        
        thread_t to_remove = thrtab::get_tid(s);
        
        if( getlock )
        {
            sock_monitor->lock();
        }
        bound_socks->yank(sock);
        if( getlock )
        {
            sock_monitor->unlock();
        }
        
        thrtab::remove(to_remove);
    }
}

void
thr_mailbox::unbind( PdFile fd, bool getlock)
{
    entity* fitem = (entity*)(file_q::file_from_desc(fd));
    if( fitem )
    {
        thread_t to_remove = thrtab::get_tid( fitem );
        
        if( getlock )
        {
            file_monitor->lock();
        }
        bound_files->yank( fd );    
        if( getlock )
        {
            file_monitor->unlock();
        }
        
        thrtab::remove(to_remove);
    }
}


bool
thr_mailbox::is_bound( PdFile fd )
{
    file_monitor->lock();
    bool ret = bound_files->contains( fd );
    file_monitor->unlock();

    return ret;
}

bool thr_mailbox::is_bound( PdSocket sock )
{
    sock_monitor->lock();
    bool ret = bound_socks->contains(sock);
    sock_monitor->unlock();

    return ret;
}

void clear_ready_sock( PDSOCKET sock )
{
	((thr_mailbox*)lwp::get_mailbox())->clear_ready( PdSocket( sock ) );
}

void clear_ready_file( PDDESC fd )
{
	((thr_mailbox*)lwp::get_mailbox())->clear_ready( PdFile( fd ) );
}

void
thr_mailbox::clear_ready( PdSocket sock )
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

void
thr_mailbox::clear_ready( PdFile fd )
{
	file_monitor->lock();
	if( ready_files->contains(fd) )
	{
		ready_files->yank(fd);
	}

	// remove any message from our messages list indicating this
	// file descriptor is ready to read
	// TODO remove either the ready_files or file_messages lists
	io_entity* ioe = file_q::file_from_desc( fd );
	if( ioe != NULL )
	{
		match_message_pred criterion(ioe->self(), MSG_TAG_FILE);
		if( file_messages->contains(&criterion) )
		{
			message* m = file_messages->yank(&criterion);
			delete m;
		}	
	}

	file_monitor->unlock();
}

class message_dumper {
  public:
    int visit(message* m) {
        m->dump("         ");
        return 1;
    }
};


class BoundItemDumper
{
public:
    int visit( PdSocket desc )
    {
        fprintf( stderr, "%d ", desc.s );
        return 1;
    }

    int visit( PdFile desc )
    {
        fprintf( stderr, "%d ", desc.fd );
        return 1;
    }
};

void thr_mailbox::dump_state() {
    message_dumper md;
    BoundItemDumper bidumper;

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
        fprintf(stderr, "   number of bound sockets: %d\n",
                    bound_socks->get_size());
        fprintf(stderr, "      bound socks are: ");
        bound_socks->visit(&bidumper);
        fprintf(stderr, "\n");
    }
    
    if(ready_socks && ready_socks->get_size()) {
        fprintf(stderr, "   number of ready sockets: %d\n",
                    ready_socks->get_size());
        fprintf(stderr, "      ready socks are: ");
        ready_socks->visit(&bidumper);
        fprintf(stderr, "\n");
    }
    
    if(bound_files && bound_files->get_size()) {
        fprintf(stderr, "   number of bound files: %d\n",
                    bound_files->get_size());
        fprintf(stderr, "      bound files are: ");
        bound_files->visit(&bidumper);
        fprintf(stderr, "\n");
    }
    
    if(ready_files && ready_files->get_size()) {
        fprintf(stderr, "   number of ready files: %d\n",
                    ready_files->get_size());
        fprintf(stderr, "      ready files are: ");
        ready_files->visit(&bidumper);
        fprintf(stderr, "\n");
    }
}

