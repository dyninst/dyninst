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

