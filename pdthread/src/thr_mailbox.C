/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include <stdio.h>
#include "common/h/Vector.h"
#include "thr_mailbox.h"
#include "thrtab.h"
#include "message.h"
#include "io_message.h"
#include "refarray.h"
#include "predicate.h"
#include "message_predicates.h"
#include "mailbox_defs.h"
#include "xplat/Mutex.h"
#include "WaitSet.h"

// /* DEBUG */ #include <pthread.h>

#if DO_DEBUG_LIBPDTHREAD_THR_MAILBOX == 1
#define DO_DEBUG_LIBPDTHREAD 1
#else
#define DO_DEBUG_LIBPDTHREAD 0
#endif

#define CURRENT_FILE thr_mailbox_C
#include "thr_debug.h"

#undef DO_DEBUG_LIBPDTHREAD

namespace pdthr
{

class SocketWaitSetPopulator : public dllist_visitor<PdSocket>
{
private:
    thr_mailbox& mbox;
    WaitSet& wset;

public:
    SocketWaitSetPopulator( thr_mailbox& _mbox, WaitSet& _wset )
      : mbox( _mbox ),
        wset( _wset )
    { }

    virtual bool visit( PdSocket s );
};

bool
SocketWaitSetPopulator::visit( PdSocket desc)
{
	if(!mbox.ready_socks->contains(desc))
    {
        thr_debug_msg(CURRENT_FUNCTION,
            "ADDING %d to wait set\n", (unsigned)desc.s);
           
        wset.Add( desc );
	}
	else
	{
		thr_debug_msg(CURRENT_FUNCTION,
            "NOT ADDING %d to wait set\n", (unsigned)desc.s);
	}

	return true;	// we always want to continue visiting list items
}


class FileWaitSetPopulator : public dllist_visitor<PdFile>
{
public:
	thr_mailbox& mbox;
    WaitSet& wset;


	FileWaitSetPopulator( thr_mailbox& _mbox, WaitSet& _wset )
	  : mbox( _mbox ),
		wset( _wset )
	{ }

    virtual bool visit( PdFile desc );
};


bool
FileWaitSetPopulator::visit( PdFile desc)
{
	if(!mbox.ready_files->contains(desc))
    {
        thr_debug_msg(CURRENT_FUNCTION,
            "ADDING %d to wait set\n", (unsigned)desc.fd);
           
        wset.Add( desc );
	}
	else
	{
		thr_debug_msg(CURRENT_FUNCTION,
            "NOT ADDING %d to wait set\n", (unsigned)desc.fd);
	}

	return true;	// we always want to continue visiting list items
}




class SocketReadySetPopulator : public dllist_visitor<PdSocket>
{
public:
    thr_mailbox& mbox;
    WaitSet& wset;

	SocketReadySetPopulator( thr_mailbox& _mbox, WaitSet& _wset )
	  : mbox( _mbox ),
		wset( _wset )
	{ }

    bool visit( PdSocket desc );
};


bool
SocketReadySetPopulator::visit( PdSocket desc )
{
	io_entity *ie = socket_q::socket_from_desc(desc);

	if(!ie || !mbox.bound_socks->contains(desc))
      return true;

	// bound_socks had better contain desc, because
	// we are visiting every item in the bound socks set
	// assert( mbox->bound_socks->contains(desc) );

	if( !mbox.ready_socks->contains(desc) )
	{
        if( wset.HasData( desc ) )
        {
			thr_debug_msg(CURRENT_FUNCTION,
                "enqueuing SOCKET message from %d\n", (unsigned)desc.s);

			message *m = new io_message(ie,ie->self(), MSG_TAG_SOCKET);
			
			mbox.put_sock(m);
			mbox.ready_socks->put(desc);
		} 

		else {
			thr_debug_msg(CURRENT_FUNCTION,
                "NOT enqueuing SOCKET message from %d\n", (unsigned)desc.s);
		}
	}
	return true;	// we always want to continue visiting list items
}


class FileReadySetPopulator : public dllist_visitor<PdFile>
{
public:
    thr_mailbox& mbox;
    WaitSet& wset;

	FileReadySetPopulator( thr_mailbox& _mbox, WaitSet& _wset )
	  : mbox( _mbox ),
		wset( _wset )
	{ }

    bool visit( PdFile desc );
};


bool
FileReadySetPopulator::visit( PdFile desc )
{
	io_entity *ie = file_q::file_from_desc(desc);

	if(!ie || !mbox.bound_files->contains(desc))
      return true;

	// bound_files had better contain desc, because
	// we are visiting every item in the bound files set
	// assert( mbox.bound_file->contains(desc) );

	if( !mbox.ready_files->contains(desc) )
	{
        if( wset.HasData( desc ) )
		{
			thr_debug_msg(CURRENT_FUNCTION,
                "enqueuing FILE message from %d\n", (unsigned)desc.fd);

			message *m = new io_message(ie,ie->self(), MSG_TAG_FILE);
			
			mbox.put_file(m);
			mbox.ready_files->put(desc);
		} 

		else {
			thr_debug_msg(CURRENT_FUNCTION,
                "NOT enqueuing FILE message from %d\n", (unsigned)desc.fd);
		}
	}
	return true;	// we always want to continue visiting list items
}


// visitor for finding invalid sockets
// Intended to be used on the bound_socks list.
class SocketBadDescFinder : public dllist_visitor<PdSocket>
{
private:
    pdvector<PdSocket> socks;

public:
    virtual bool visit(PdSocket desc)
    {
        if( desc.IsBad() )
        {
            socks.push_back( desc );
        }
        return true;        // always continue search
    }

    const pdvector<PdSocket>& GetSocks( void ) const    { return socks; }
};


class FileBadDescFinder : public dllist_visitor<PdFile>
{
private:
    pdvector<PdFile> fds;

public:
    virtual bool visit(PdFile desc)
    {
        if( desc.IsBad() )
        {
            fds.push_back( desc );
        }
        return true;        // always continue search
    }

    const pdvector<PdFile>& GetFiles( void ) const  { return fds; }
};


class SocketReadyBufferedVisitor : public dllist_visitor<PdSocket>
{
private:
	io_entity* ready_ioe;

public:
	SocketReadyBufferedVisitor( void )
	  : ready_ioe( NULL )
	{ }

	virtual bool visit( PdSocket desc );
    const io_entity* GetReadyEntity( void ) const   { return ready_ioe; }
};




// TODO this method of checking for ready buffered entities is not necessarily
// fair - we always start visiting from the list's beginning
bool
SocketReadyBufferedVisitor::visit( PdSocket desc )
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



void
thr_mailbox::populate_wait_set( WaitSet* wset )
{
    // ...add our bound sockets that have not
    // already been indicated as "ready-to-read"
    SocketWaitSetPopulator sock_pop( *this, *wset );
    sockq_mutex.Lock();
    bound_socks->visit( &sock_pop );
    sockq_mutex.Unlock();

    // ...add our bound files that have not already
    // been indicated as "ready-to-read"
    FileWaitSetPopulator file_pop( *this, *wset );
    fileq_mutex.Lock();
    bound_files->visit( &file_pop );
    fileq_mutex.Unlock();
}


void
thr_mailbox::handle_wait_set_input( WaitSet* wset )
{
    // check each of our bound sockets to see if it has data available
    // if a socket is ready to read, add it to the "ready-to-read" set
    SocketReadySetPopulator sock_pop( *this, *wset );
    sockq_mutex.Lock();
    bound_socks->visit( &sock_pop );
    sockq_mutex.Unlock();

    // check each of our bound files to see if it has data available
    // if a file is ready to read, add it to the "ready-to-read" set
    FileReadySetPopulator file_pop( *this, *wset );
    fileq_mutex.Lock();
    bound_files->visit( &file_pop );
    fileq_mutex.Unlock();
}




void
thr_mailbox::wait_for_input( pollcallback_t pcb )
{
    unsigned int i;
    assert(thr_type(owned_by) == item_t_thread);

    thr_debug_msg(CURRENT_FUNCTION, "ENTERING for mailbox owned by %d\n",
        owned_by);

	// allocate a set to hold the sockets that we will check
    WaitSet* wset = WaitSet::BuildWaitSet();
    
    if (pcb)
      wset->RegisterCallback(pcb);

    bool done = false;
    while( !done )
	{
        thr_debug_msg(CURRENT_FUNCTION, "LOOPING for mailbox owned by %d\n",
                        owned_by);

        // populate the WaitSet with the set of objects (sockets,
        // files, pipes, etc.) we will be blocking on
        wset->Clear();
        populate_wait_set( wset );

        thr_debug_msg(CURRENT_FUNCTION,
                        "SELECTING for mailbox owned by %d\n",
                        owned_by );
        
        // do the wait
        WaitSet::WaitReturn wstatus = wset->Wait();

        thr_debug_msg(CURRENT_FUNCTION,
            "SELECT RETURNING for mailbox owned by %d; status = %d\n",
            owned_by, wstatus);

        if( wstatus == WaitSet::WaitInput )
        {
            // we think there is some input available
            handle_wait_set_input( wset );

			// if we popped out of the select because someone has
			// put a message into our message queue, we have nothing else
			// to do here - our caller will notice that there is a message
			// available in the message queues
			done = true;
        }
        else if( wstatus == WaitSet::WaitError )
        {
            // We might have at least one bad file descriptor in our set of
            // bound descriptors -
            // (sadly, this can happen when a client socket connection is
            // broken without unbinding the bad descriptor first)
            // check them all and unbind the bad descriptor
            // note that we don't unbind them as part of the visit
            // to avoid changing bound_socks from underneath the
            // visit method.

            // check sockets
            SocketBadDescFinder sock_bdf;
            sockq_mutex.Lock();
            bound_socks->visit( &sock_bdf );

            // unbind the bad descriptors
            const pdvector<PdSocket>& socks = sock_bdf.GetSocks();
            for( i = 0; i < socks.size(); i++ )
            {
                // note we already have the lock
                unbind( socks[i], false ); 
            }
            sockq_mutex.Unlock();

            // check files
            FileBadDescFinder file_bdf;
            fileq_mutex.Lock();
            bound_files->visit( &file_bdf );

            // unbind the bad descriptors
            const pdvector<PdFile>& fds = file_bdf.GetFiles();
            for( i = 0; i < fds.size(); i++ )
            {
                // note we already have the lock
                unbind( fds[i], false ); 
            }
            fileq_mutex.Unlock();
        }
        else if( wstatus == WaitSet::WaitTimeout )
        {
			// spurious wakeup - we shouldn't have woken up,
			// because we have an infinite timeout
        	thr_debug_msg(CURRENT_FUNCTION,
                "SELECT spurious wakeup for mailbox owned by %d; status = %d\n",
                owned_by, wstatus);
        }
        else
        {
            // unrecognized wait return value
            assert( false );
        }
    }
	delete wset;
}




thr_mailbox::thr_mailbox(thread_t owner) 
  : mailbox(owner),
    messages( new dllist<message*,XPlat::Monitor> ),
    sock_messages( new dllist<message*,XPlat::Monitor> ),
    file_messages( new dllist<message*,XPlat::Monitor> ),
    bound_socks( new dllist<PdSocket,XPlat::Monitor> ),
    ready_socks( new dllist<PdSocket,XPlat::Monitor> ),
    bound_files( new dllist<PdFile,XPlat::Monitor> ),
    ready_files( new dllist<PdFile,XPlat::Monitor> )
{
    thr_debug_msg(CURRENT_FUNCTION, "building mailbox for %d\n", owner);
    assert(owned_by == owner);
}

thr_mailbox::~thr_mailbox( void )
{
    delete messages;

    delete sock_messages;
    delete ready_socks;
    delete bound_socks;

    delete file_messages;
    delete bound_files;
    delete ready_files;
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
			SocketReadyBufferedVisitor rbv;
			bound_socks->visit( &rbv );
            const io_entity* ready_ioe = rbv.GetReadyEntity();
			if( ready_ioe != NULL )
			{
				is_someone_ready = true;
				*sender = ready_ioe->self();
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
		       unsigned io_first,
 		       pollcallback_t pcb)
{
    bool found = false;
    thread_t actual_sender = 0;
    tag_t actual_type = 0;

    match_message_pred criterion( * sender, * type );
    dllist<message*,XPlat::Monitor> * yank_from = NULL;

	/* Spin-lock for now.  What we _should_ do is convert
	   qmutex into a conditional variable raised by put().
	   
	   Also note that if one queue receive were ever to block
	   that no other receiver would be able to receieve, even
	   on other queues, because we're holding the qmutex lock
	   when we try a receive. */
	do {	   
    	actual_sender = 0;
	    actual_type = 0;
	    
	    qmutex.Lock();
	    clear_msg_avail();
	    
	    if( io_first ) {
	    	/* Special case; presumambly to make sure sockets don't time-out. */
			found = is_buffered_special_ready( sender, type );

			if( found ) {
				criterion.actual_sender = *sender;
				criterion.actual_type = *type;

				// make sure that if our previous call showed we have data
				// available, that we don't mistakenly assume that the
				// data incoming on the underlying socket that was used to
				// fill the buffer is still available
				if( sock_messages->contains( & criterion ) ) {
					yank_from = sock_messages;
					}
	            else if( file_messages->contains( & criterion ) ) {
	                yank_from = file_messages;
		            }
				} /* end if we didn't find any buffered data */
			else {
				// we have no buffered data, so check whether any
				// of our bound descriptors are ready to read
				if( (found = sock_messages->contains( & criterion )) == true ) {
					yank_from = sock_messages;
					}
				else if( (found = file_messages->contains( & criterion )) == true ) {
	                yank_from = file_messages;
    		        }
            	else if( (found = messages->contains( & criterion )) == true ) {
	                yank_from = messages;
					}
				} /* end if we have no buffered data */
			} /* end if we're checking I/O first */
		else {
			found = ( messages->contains( & criterion ) );
			if( found ) {
				yank_from = messages;
				}
			else {
				// check if any buffered special socket still has
				// input to be consumed
				found = is_buffered_special_ready( sender, type );
				if( found ) {
					criterion.actual_sender = *sender;
					criterion.actual_type = *type;

					// make sure that if our previous call showed we have data
					// available, that we don't mistakenly assume that the
					// data incoming on the underlying socket that was used to
					// fill the buffer is still available
					if( (found = sock_messages->contains( & criterion )) == true ) {
						yank_from = sock_messages;
						}
        	        else if( (found = file_messages->contains( & criterion )) == true ) {
	                    yank_from = file_messages;
    		            }
					} /* end if we found buffered data */
				else {
					// we have no buffered data, so check
					// whether any of our bound descriptors are ready to read
					if( (found = sock_messages->contains( & criterion )) == true ) {
						yank_from = sock_messages;
						}
    	            else if( (found = file_messages->contains( & criterion )) == true ) {
	                    yank_from = file_messages;
		                }
					} /* end if we did not find buffered data */
				} /* end if we didn't find non-I/O data. */
			} /* end if we're not checking I/O first */

	    actual_sender = criterion.actual_sender;
	    actual_type = criterion.actual_type;

		// /* DEBUG */ fprintf( stderr, "%s[%d]: found = %d, do_yank = %d, m = %p, yank_from = %p (%d)\n", __FILE__, __LINE__, found, do_yank, m, yank_from, pthread_self() );
		if( found ) {
	    	if( do_yank && m != NULL && yank_from != NULL ) {
    	        * m = yank_from->yank(&criterion);
				}
			qmutex.Unlock();
			break;
			}
			
		qmutex.Unlock();
		
		if( ! found && do_block ) {
			thr_debug_msg(CURRENT_FUNCTION, "blocking; size of messages = %d, size of sock_messages = %d\n", messages->get_size(), sock_messages->get_size());
			wait_for_input( pcb );
			}
		} while( do_block );
    
	if( found ) {
		if( actual_sender ) { * sender = actual_sender; }
        if( actual_type ) { * type = actual_type; }
		}
    
    return found;
	} /* end check_for() */


int thr_mailbox::put(message* m) {
    messages->put(m);
	raise_msg_avail();
    return THR_OKAY;
}

int thr_mailbox::put_sock(message* m) {
    sock_messages->put(m);
    raise_msg_avail();    
    return THR_OKAY;
}

int
thr_mailbox::put_file(message* m)
{
    file_messages->put(m);
    raise_msg_avail();    
    return THR_OKAY;
}


int thr_mailbox::recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp) {
    int retval = THR_OKAY;
    COLLECT_MEASUREMENT(THR_MSG_TIMER_START);
        
    thr_debug_msg(CURRENT_FUNCTION, "RECEIVING: size of messages = %d, size of sock_messages = %d\n", messages->get_size(), sock_messages->get_size());

    message* to_recv = NULL;
    bool did_recv = check_for(sender, tagp, true, true, &to_recv);

    thr_debug_msg(CURRENT_FUNCTION, "DONE RECEIVING: size of messages = %d, size of sock_messages = %d\n", messages->get_size(), sock_messages->get_size());

    if(!did_recv) {
        retval = THR_ERR;
        goto done;
    }
    
    *sender = to_recv->deliver(tagp, buf, countp);

    delete to_recv;

  done:
    COLLECT_MEASUREMENT(THR_MSG_TIMER_STOP);

    return retval;
}


int thr_mailbox::recv(thread_t* sender, tag_t* tagp, void** buf) {
    int retval = THR_OKAY;
    COLLECT_MEASUREMENT(THR_MSG_TIMER_START);
        
    thr_debug_msg(CURRENT_FUNCTION, "RECEIVING: size of messages = %d, size of sock_messages = %d\n", messages->get_size(), sock_messages->get_size());

    message* to_recv = NULL;
    bool did_recv = check_for(sender, tagp, true, true, &to_recv);

    thr_debug_msg(CURRENT_FUNCTION, "DONE RECEIVING: size of messages = %d, size of sock_messages = %d\n", messages->get_size(), sock_messages->get_size());

    if(!did_recv) {
        retval = THR_ERR;
        goto done;
    }
    
    *sender = to_recv->deliver(tagp, buf);

    delete to_recv;

done:
    COLLECT_MEASUREMENT(THR_MSG_TIMER_STOP);

    return retval;
}



int thr_mailbox::poll(thread_t* from, tag_t* tagp, unsigned block,
                      unsigned fd_first, pollcallback_t pcb)
{ 
    COLLECT_MEASUREMENT(THR_MSG_TIMER_START);

    bool found = check_for(from, tagp, (block != 0), false, NULL, fd_first);

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
    sockq_mutex.Lock();
    bound_socks->put(sock);
    sockq_mutex.Unlock();
}

void
thr_mailbox::bind( PdFile desc,
                        unsigned int special,
                        int (*wb)(void*),
                        void* arg,
                        thread_t* ptid )
{
    *ptid = thrtab::create_file( desc, owned_by, wb, arg, (special != 0) );
    fileq_mutex.Lock();
    bound_files->put( desc );
    fileq_mutex.Unlock();
}

void
thr_mailbox::unbind( PdSocket sock, bool getlock )
{
    entity* s = (entity*)(socket_q::socket_from_desc(sock));
    if (s) {
        
        thread_t to_remove = thrtab::get_tid(s);
        
        if( getlock )
        {
            sockq_mutex.Lock();
        }
        bound_socks->yank(sock);
        if( getlock )
        {
            sockq_mutex.Unlock();
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
            fileq_mutex.Lock();
        }
        bound_files->yank( fd );    
        if( getlock )
        {
            fileq_mutex.Unlock();
        }
        
        thrtab::remove(to_remove);
    }
}


bool
thr_mailbox::is_bound( PdFile fd )
{
    fileq_mutex.Lock();
    bool ret = bound_files->contains( fd );
    fileq_mutex.Unlock();

    return ret;
}

bool thr_mailbox::is_bound( PdSocket sock )
{
    sockq_mutex.Lock();
    bool ret = bound_socks->contains(sock);
    sockq_mutex.Unlock();

    return ret;
}

void
thr_mailbox::clear_ready( PdSocket sock )
{
	sockq_mutex.Lock();
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

	sockq_mutex.Unlock();
}

void
thr_mailbox::clear_ready( PdFile fd )
{
	fileq_mutex.Lock();
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

	fileq_mutex.Unlock();
}

class message_dumper : public dllist_visitor<message*> 
{
public:
    virtual bool visit(message* m)
    {
        m->dump("         ");
        return true;
    }
};


class SocketBoundItemDumper : public dllist_visitor<PdSocket>
{
public:
    virtual bool visit( PdSocket desc )
    {
        fprintf( stderr, "%d ", desc.s );
        return true;
    }
};

class FileBoundItemDumper : public dllist_visitor<PdFile>
{
public:
    virtual bool visit( PdFile desc )
    {
        fprintf( stderr, "%d ", desc.fd );
        return true;
    }
};

void thr_mailbox::dump_state() {
    message_dumper md;
    SocketBoundItemDumper sockdumper;
    FileBoundItemDumper filedumper;

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
        bound_socks->visit(&sockdumper);
        fprintf(stderr, "\n");
    }
    
    if(ready_socks && ready_socks->get_size()) {
        fprintf(stderr, "   number of ready sockets: %d\n",
                    ready_socks->get_size());
        fprintf(stderr, "      ready socks are: ");
        ready_socks->visit(&sockdumper);
        fprintf(stderr, "\n");
    }
    
    if(bound_files && bound_files->get_size()) {
        fprintf(stderr, "   number of bound files: %d\n",
                    bound_files->get_size());
        fprintf(stderr, "      bound files are: ");
        bound_files->visit(&filedumper);
        fprintf(stderr, "\n");
    }
    
    if(ready_files && ready_files->get_size()) {
        fprintf(stderr, "   number of ready files: %d\n",
                    ready_files->get_size());
        fprintf(stderr, "      ready files are: ");
        ready_files->visit(&filedumper);
        fprintf(stderr, "\n");
    }
}

} // namespace pdthr

void clear_ready_sock( PDSOCKET sock )
{
	((pdthr::thr_mailbox*)pdthr::lwp::get_mailbox())->clear_ready( PdSocket( sock ) );
}

void clear_ready_file( PDDESC fd )
{
	((pdthr::thr_mailbox*)pdthr::lwp::get_mailbox())->clear_ready( PdFile( fd ) );
}

