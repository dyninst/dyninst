#include "pdthread/h/thread.h"
#include "pdthread/src/WaitSet-win.h"
#include <windows.h>


#define CURRENT_FILE WaitSet_win_C
#include "thr_debug.h"

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
    readyFileIdx( -1 )
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
}


WaitSet::WaitReturn
WinWaitSet::Wait( void )
{
    WaitSet::WaitReturn ret = WaitSet::WaitNone;

    if( check_wmsg_q || (socks.size() > 0) || (files.size() > 0) )
    {
        DWORD nWaitObjs = socks.size() + files.size();

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
                    // it was an Event associated with a socket
                    readySockIdx = (int)ndx;
                }
                else
                {
                    // it was a file
                    readyFileIdx = (int)(ndx - socks.size());
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
    }
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

