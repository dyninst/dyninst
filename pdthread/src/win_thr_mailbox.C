#include "pdthread/src/win_thr_mailbox.h"
#include "pdthread/src/WaitSet-win.h"
#include "pdthread/src/thrtab.h"
#include "pdthread/src/io_message.h"

namespace pdthr
{

void
win_thr_mailbox::populate_wait_set( WaitSet* wset )
{
    // allow base class to populate
    thr_mailbox::populate_wait_set( wset );

    // add the Windows message queue if necessary
    if( bound_tid != THR_TID_UNSPEC )
    {
        WinWaitSet* wwset = (WinWaitSet*)wset;
        wwset->AddWmsgQueue();
    }
}


void
win_thr_mailbox::handle_wait_set_input( WaitSet* wset )
{
    WinWaitSet* wwset = (WinWaitSet*)wset;

    // we've been given indication that there is some input available

    // first check whether it was due to the Windows msg
    if( wwset->WmsgQueueHasData() )
    {
        assert( bound_tid != THR_TID_UNSPEC );

        // generate a message from the wmsg queue
        wmsg_q* wmsgQueueEntity = (wmsg_q*)thrtab::get_entry( bound_tid );
        message* m = new io_message( wmsgQueueEntity, bound_tid, MSG_TAG_WMSG );
        put( m );
    }

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


} // namespace pdthr

