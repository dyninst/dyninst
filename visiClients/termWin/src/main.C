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

// $Id: main.C,v 1.14 2004/12/07 20:48:08 tlmiller Exp $

#include <stdio.h>
#include "common/h/headers.h"
#include "common/h/Ident.h"

#if defined(os_windows)
#include "visiClients/termWin/src/TermWinWinTkGUI.h"
#else
#include "visiClients/termWin/src/TermWinUnixTkGUI.h"
#endif // defined(os_windows)
#include "visiClients/termWin/src/TermWinCLUI.h"

extern "C" const char V_libpdutil[];
Ident V_PdutilUid(V_libpdutil,"Paradyn");
extern "C" const char V_libpdthread[];
Ident V_PdthreadUid(V_libpdthread,"Paradyn");
extern "C" const char V_termWin[];
Ident V_TermWinUid(V_termWin,"Paradyn");


int 
main(int argc, char **argv) 
{
    int ret = 0;
    bool useGUI = true;           // normally we want to use a GUI

    if( getenv( "TERMWIN_DEBUG" ) != NULL )
    {
        fprintf( stderr, "termWin: waiting for debugger, pid=%d\n", getpid() );
        volatile bool bCont = false;
        while( !bCont )
        {
            // spin!
        }
    }

    // start our own process group
    (void)setsid();
    

    // check and parse the command line
    if( (argc < 2) || (argc > 3) )
    {
        exit( -1 );
    }
    if( argc == 3 )
    {
        if( !strcmp( argv[2], "-cl" ) )
        {
            useGUI = false;
        }
    }

    // determine the socket that the front-end set up for us
    PDSOCKET serv_sock = atoi(argv[1]);

    // build the desired type of termWin server object
    TermWin* tw = NULL;
    if( useGUI )
    {
#if defined(os_windows)
        tw = new TermWinWinTkGUI( argv[0], serv_sock, fileno(stdin) );
#else
        tw = new TermWinUnixTkGUI( argv[0], serv_sock, fileno(stdin) );
#endif // defined(os_windows)
    }
    else
    {
        tw = new TermWinCLUI( serv_sock, fileno(stdin) );
    }

    // initialize our TermWin server
    if( !tw->Init() )
    {
        exit( -1 );
    }

    // handle events
    while( !tw->IsDoneHandlingEvents() )
    {
        bool ok;


        // let object perform any per-iteration activity
        ok = tw->DoPendingWork();
        if( !ok )
        {
            ret = -1;
            break;
        }

        // peek at the next event
        thread_t mtid = THR_TID_UNSPEC;
        tag_t mtag = MSG_TAG_ANY;
        int err = msg_poll( &mtid, &mtag, 1 );      // 1 -> non-blocking poll
        if( err == THR_ERR )
        {
            tw->ShowError( "unable to peek at next event" );
            ret = -1;
            break;
        }

        // handle the event
        if( !tw->DispatchEvent( mtid, mtag ) )
        {
        	if( ! tw->IsDoneHandlingEvents() ) { continue; }
            tw->ShowError( "failed to handle event" );
            ret = -1;
            break;
        }
    }

    // cleanup
    delete tw;

    return ret;
}




