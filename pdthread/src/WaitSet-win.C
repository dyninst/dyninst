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

#include "pdthread/h/thread.h"
#include "pdthread/src/WaitSet-win.h"
#include <windows.h>


#define CURRENT_FILE WaitSet_win_C
#include "thr_debug.h"

extern thread_t PCthreadTid;
extern thread_t DMthreadTid;

namespace pdthr
{


WaitSet*
WaitSet::BuildWaitSet( void )
{
    return new WinWaitSet;
}



WinWaitSet::WinWaitSet( void )
  : check_wmsg_q( false ),
    wmsg_q_hasdata( false ),
    readySockIdx( -1 ),
    readyFileIdx( -1 ),
    hMsgAvailEvent( INVALID_HANDLE_VALUE )
{
    // nothing else to do
}


WinWaitSet::~WinWaitSet( void )
{
    // nothing else to do
}


void
WinWaitSet::Clear( void )
{
    socks.clear();
    files.clear();
    check_wmsg_q = false;

    wmsg_q_hasdata = false;
    readySockIdx = -1;
    readyFileIdx = -1;

    hMsgAvailEvent = INVALID_HANDLE_VALUE;
}


WaitSet::WaitReturn
WinWaitSet::Wait( void )
{
    WaitSet::WaitReturn ret = WaitSet::WaitNone;

        DWORD nWaitObjs = socks.size() + files.size() + 1;

        // build the set of handles to wait over
        HANDLE* waitObjs = new HANDLE[nWaitObjs];

        // first add handles for events associated with sockets
        // note that WSAEventSelect sets a socket to non-blocking mode;
        // we reset all sockets to blocking mode at the end of this function.
        unsigned int curIdx = 0;
        for( pdvector<PdSocket>::iterator sockiter = socks.begin();
                sockiter != socks.end();
                sockiter++ )
        {
            // build an event associated with this socket
            HANDLE hEvent = WSACreateEvent();
            if( hEvent == WSA_INVALID_EVENT )
            {
                // indicate the error
                thr_debug_msg(CURRENT_FUNCTION,
                            "failed to create Event for socket events: %08x\n",
                            WSAGetLastError() );
                delete[] waitObjs;
                return WaitError;
            }

            // indicate the events we're interested in
            int sret = WSAEventSelect( sockiter->s,
                                hEvent,
                                FD_CONNECT | FD_ACCEPT | FD_READ | FD_CLOSE );
            if( sret == SOCKET_ERROR )
            {
                thr_debug_msg(CURRENT_FUNCTION,
                        "failed to set events of interest on Event: %08x\n",
                        WSAGetLastError() );
                delete[] waitObjs;
                return WaitError;
            }

            waitObjs[curIdx] = hEvent;
            curIdx++;
        }

        // now add handles for files
        // unlike sockets, we can wait on the file handle itself
        for( pdvector<PdFile>::iterator fileiter = files.begin();
                fileiter != files.end();
                fileiter++ )
        {
            waitObjs[curIdx] = fileiter->fd;
            curIdx++;
        }

        // add handle for msg-available event
        assert( hMsgAvailEvent != INVALID_HANDLE_VALUE );
        waitObjs[curIdx] = hMsgAvailEvent;
        curIdx++;

        // wait for available input
        DWORD waitRet;
        if( check_wmsg_q )
        {
            waitRet = MsgWaitForMultipleObjects( nWaitObjs,
                                                    waitObjs,
                                                    FALSE,
                                                    INFINITE,
                                                    QS_ALLINPUT );
        }
        else
        {
            waitRet = WaitForMultipleObjects( nWaitObjs,
                                                waitObjs,
                                                FALSE,
                                                INFINITE );
        }

        // determine why we regained control
        if( (waitRet >= WAIT_OBJECT_0) && 
            (waitRet <= (WAIT_OBJECT_0 + nWaitObjs)) )
        {
            // we've got input
            ret = WaitSet::WaitInput;

            if( waitRet == (WAIT_OBJECT_0 + nWaitObjs) )
            {
                // there's a message in the message queue
                wmsg_q_hasdata = true;
            }
            else
            {
                // one of the objects was signalled
                // figure out which one, and indicate it
                // remember that we put Events for sockets in the array first,
                // then the handles for the files
                unsigned int ndx = waitRet - WAIT_OBJECT_0;

                if( ndx < socks.size() )
                {
                    // It was an Event associated with a socket.
                    // Find out what type of event caused us to un-block
                    // (This has the side effect of clearing the state
                    // in the socket that caused us to pop out of the wait.
                    // If we don't do this, next time around we don't wait
                    // even if there is no input available on the socket.)
                    WSANETWORKEVENTS netEvents;
                    if( WSAEnumNetworkEvents( socks[ndx].s,
                                                NULL,
                                                &netEvents ) != SOCKET_ERROR )
                    {
                        readySockIdx = (int)ndx;
                    }
                }
                else if( ndx < (socks.size() + files.size()) )
                {
                    // it was a file
                    readyFileIdx = (int)(ndx - socks.size());
                }
                else
                {
                    assert( ndx == (nWaitObjs - 1) );
#if READY
                    if( thr_self() == PCthreadTid )
                    {
                        fprintf( stderr, "PC: received msg\n" );
                    }

                    if( thr_self() == DMthreadTid )
                    {
                        fprintf( stderr, "DM: received msg\n" );
                    }
#endif // READY
                }
            }
        }
        else if( waitRet == WAIT_TIMEOUT )
        {
            // the call timed out
            ret = WaitTimeout;
        }
        else if( waitRet == (DWORD)(-1) )
        {
            // the call failed
            ret = WaitError;
        }
        else
        {
            // we've seen a return value we don't handle -
            // possibly an abandoned mutex
            thr_debug_msg(CURRENT_FUNCTION,
                "Unrecognized MsgWaitforMultipleObjects return value\n" );
            abort();
        }

        // reset all sockets to blocking mode
        curIdx = 0;
        for( pdvector<PdSocket>::iterator siter = socks.begin();
                siter != socks.end();
                siter++ )
        {
            // reset socket to blocking mode
            // recall that we put sockets first in the waitObjs array
            if( WSAEventSelect( siter->s, waitObjs[curIdx], 0 ) == SOCKET_ERROR )
            {
                thr_debug_msg(CURRENT_FUNCTION,
                        "failed to reset events of interest for socket: %08x\n",
                        WSAGetLastError() );
            }
            else
            {
                u_long block = 0;
                if( ioctlsocket( siter->s, FIONBIO, &block ) == SOCKET_ERROR )
                {
                    thr_debug_msg(CURRENT_FUNCTION,
                        "failed to reset socket to blocking mode: %08x\n",
                        WSAGetLastError() );
                }
            }
            WSACloseEvent( waitObjs[curIdx] );

            curIdx++;
        }

        delete[] waitObjs;
    return ret;
}


bool
WinWaitSet::HasData( const PdSocket& s )
{
    bool ret = false;

    if( readySockIdx != -1 )
    {
        if( socks[readySockIdx].s == s.s )
        {
            ret = true;
        }
    }
    return ret;
}


bool
WinWaitSet::HasData( const PdFile& f )
{
    bool ret = false;
    if( readyFileIdx != -1 )
    {
        if( files[readyFileIdx].fd == f.fd )
        {
            ret = true;
        }
    }
    return ret;
}


} // namespace pdthr

