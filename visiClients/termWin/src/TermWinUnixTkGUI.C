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

// $Id: TermWinUnixTkGUI.C,v 1.2 2004/06/21 22:06:55 pcroth Exp $
// $Id: TermWinUnixTkGUI.C,v 1.2 2004/06/21 22:06:55 pcroth Exp $

#include <assert.h>
#include "tcl.h"
#include "tk.h"
#include "pdthread/h/thread.h"
#include "visiClients/termWin/src/TermWinUnixTkGUI.h"



bool
TermWinUnixTkGUI::Init( void )
{
    // initialize base class
    if( !TermWinTkGUI::Init() )
    {
        return false;
    }

    // ensure that the thread library is aware of available input
    // on our XDR connections
    rpcSockCallback += (RPCSockCallbackFunc)clear_ready_sock;

    assert( tkTid == THR_TID_UNSPEC );
    Display* UIMdisplay = Tk_Display( Tk_MainWindow( GetInterp() ) );
    int xfd = XConnectionNumber( UIMdisplay );

    int retVal = msg_bind_socket( xfd,
                                1, // "special" flag --> libthread leaves it
                                   // to us to manually dequeue these messages
                                NULL,
                                NULL,
                                &tkTid );
    if( retVal != THR_OKAY )
    {
        return false;
    }

    return true;
}




bool
TermWinUnixTkGUI::DispatchEvent( thread_t mtid, tag_t mtag )
{
    bool ret = true;


    if( (mtag == MSG_TAG_SOCKET) && (mtid == tkTid) )
    {
        // there is work to be done to service the GUI
        DoPendingWork();

        // indicate to the thread library we've consumed input
        // from our X connection
        clear_ready_sock( thr_socket( tkTid ) );
    }
    else
    {
        ret = TermWinTkGUI::DispatchEvent( mtid, mtag );
    }

    return ret;
}


