// $Id: TermWinUnixTkGUI.C,v 1.1 2004/06/21 18:59:04 pcroth Exp $

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


