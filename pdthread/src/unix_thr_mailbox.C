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
#include "pdthread/src/unix_thr_mailbox.h"
#include "pdthread/src/WaitSet-unix.h"

#if DO_DEBUG_LIBPDTHREAD_THR_MAILBOX == 1
#define DO_DEBUG_LIBPDTHREAD 1
#else
#define DO_DEBUG_LIBPDTHREAD 0
#endif

#define CURRENT_FILE unix_thr_mailbox_C
#include "thr_debug.h"

#undef DO_DEBUG_LIBPDTHREAD
namespace pdthr
{


void
unix_thr_mailbox::populate_wait_set( WaitSet* wset )
{
    // allow base class to populate
    thr_mailbox::populate_wait_set( wset );

    // add the "message available" pipe
    wset->Add( msg_avail_pipe.GetReadEndpoint() );
}


void
unix_thr_mailbox::raise_msg_avail( void )
{
    if( !msg_avail_pipe.RaiseMsgAvailable() )
    {
        thr_debug_msg(CURRENT_FUNCTION,
            "write to msg_avail pipe would've blocked\n" );
    }
}    

void
unix_thr_mailbox::clear_msg_avail( void )
{
    if( !msg_avail_pipe.Drain() )
    {
        // there was an error
        thr_debug_msg(CURRENT_FUNCTION, "failed to drain msg avail pipe\n" );
    }
}


} // namespace pdthr

