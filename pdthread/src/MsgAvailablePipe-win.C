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

#include <windows.h>
#include "pdthread/src/MsgAvailablePipe.h"

namespace pdthr
{

MsgAvailablePipe::MsgAvailablePipe( void )
{
    HANDLE hread = INVALID_HANDLE_VALUE;
    HANDLE hwrite= INVALID_HANDLE_VALUE;

    // try to create the pipe
    if( CreatePipe( &hread, &hwrite, NULL, 0 ) )
    {
        // set the write end to be non-blocking
        //
        // A non-blocking pipe is a way to avoid the deadlock that
        // occurs in case the sender sends so many messages that it
        // fills the pipe causing the sender to block writing into the pipe
        // (and holding the receiver's mailbox's mutex), while the receiver
        // is blocked waiting to acquire the mutex in order to consume
        // the bytes from the pipe.
        // 
        DWORD dwNewState = PIPE_NOWAIT;
        if( SetNamedPipeHandleState( hwrite, &dwNewState, NULL, NULL ) )
        {
            // save the pipe's endpoints
            rfd = PdFile( hread );
            wfd = PdFile( hwrite );
        }
        else
        {
            CloseHandle( hread );
            CloseHandle( hwrite );
        }
    }
}

MsgAvailablePipe::~MsgAvailablePipe( void )
{
    CloseHandle( rfd.fd );
    CloseHandle( wfd.fd );
}



bool
MsgAvailablePipe::Drain( void )
{
    bool ret = true;

    bool done = false;
    while( !done )
    {
        DWORD nBytesAvailable = 0;
        BOOL bPeekRet = PeekNamedPipe( rfd.fd,
                                        NULL,
                                        0,
                                        NULL,
                                        &nBytesAvailable,
                                        NULL );
        if( bPeekRet && (nBytesAvailable == 0) )
        {
            // the pipe has been drained
            done = true;
        }
        else if( bPeekRet )
        {
            // there is data available to be read
            static char buf[1024];
            DWORD nBytesToRead = 1024;
            DWORD nBytesRead = 0;
            BOOL bReadRet = ReadFile( rfd.fd,
                                        buf,
                                        nBytesToRead, 
                                        &nBytesRead,
                                        NULL );
            if( !bReadRet )
            {
                // the read failed
                done = true;
                ret = false;
            }
        }
    }

    return ret;
}


bool
MsgAvailablePipe::RaiseMsgAvailable( void )
{
    bool ret = true;

    static char s = '0';
    DWORD nWritten = 0;
    BOOL bRet = WriteFile( wfd.fd, &s, sizeof(s), &nWritten, NULL );
    if( !bRet || (nWritten == 0) )
    {
        // The write failed.
        // The pipe is full if bRet == TRUE && nWritten == 0.
        // The write failed for some other reason if bRet == FALSE.
        ret = false;
    }
    return ret;
}

} // namespace pdthr

