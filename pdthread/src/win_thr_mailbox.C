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

#include "pdthread/src/win_thr_mailbox.h"
#include "pdthread/src/WaitSet-win.h"
#include "pdthread/src/thrtab.h"
#include "pdthread/src/io_message.h"

namespace pdthr
{

win_thr_mailbox::win_thr_mailbox(thread_t owner)
  : thr_mailbox( owner ),
    bound_tid( THR_TID_UNSPEC ),
    hMsgAvailableEvent( CreateEvent( NULL,      // default security attrs
                                        TRUE,   // manual reset
                                        FALSE,  // initially nonsignaled
                                        NULL ))  // unnamed
{
    // nothing else to do
}


win_thr_mailbox::~win_thr_mailbox( void )
{
    CloseHandle( hMsgAvailableEvent );
}



void
win_thr_mailbox::populate_wait_set( WaitSet* wset )
{
    // allow base class to populate
    thr_mailbox::populate_wait_set( wset );

    WinWaitSet* wwset = (WinWaitSet*)wset;

    // add the msg available event
    wwset->AddMsgAvailableEvent( hMsgAvailableEvent );

    // add the Windows message queue if necessary
    if( bound_tid != THR_TID_UNSPEC )
    {
        wwset->AddWmsgQueue();
    }

}


void
win_thr_mailbox::handle_wait_set_input( WaitSet* wset )
{
    WinWaitSet* wwset = (WinWaitSet*)wset;

    // we popped out of a wait - figure out why

    // check whether it was due to the Windows msg
    if( wwset->WmsgQueueHasData() )
    {
        assert( bound_tid != THR_TID_UNSPEC );

        // generate a message from the wmsg queue
        wmsg_q* wmsgQueueEntity = (wmsg_q*)thrtab::get_entry( bound_tid );
        message* m = new io_message( wmsgQueueEntity, bound_tid, MSG_TAG_WMSG );
        put( m );
    }

    // no need to check if it was due to an inter-thread message - 
    // the sender puts this message directly into our queue

    // allow base class to check for input
    thr_mailbox::handle_wait_set_input( wset );
}


void
win_thr_mailbox::bind_wmsg( thread_t* ptid )
{
    bound_tid = thrtab::create_wmsg( owned_by );
    *ptid  = bound_tid;
}


void
win_thr_mailbox::unbind_wmsg( void )
{
    if( bound_tid != THR_TID_UNSPEC )
    {
        thrtab::remove( bound_tid );
        bound_tid = THR_TID_UNSPEC;
    }
}


void
win_thr_mailbox::raise_msg_avail( void )
{
    SetEvent( hMsgAvailableEvent );    
}

void
win_thr_mailbox::clear_msg_avail( void )
{
    ResetEvent( hMsgAvailableEvent );
}

} // namespace pdthr

