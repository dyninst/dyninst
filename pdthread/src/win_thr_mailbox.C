#include "win_thr_mailbox.h"
#include "WaitSet-win.h"
#include "thrtab.h"

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
#if READY
        thread_t tid = bound_tid;
        tag_t = MSG_TAG_WMSG;

        message* m = new io_message( ie, ie->self(), MSG_TAG_WMSG );
        mbox.put_sock( m );
        mbox.ready_socks->put(desc);
#endif // READY
    }

    // allow base class to check for input
    thr_mailbox::handle_wait_set_input( wset );
}


void
win_thr_mailbox::bind_wmsg( thread_t* ptid )
{
    bound_tid = thrtab::create_wmsg();
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

