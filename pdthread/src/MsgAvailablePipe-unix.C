#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include "pdthread/src/MsgAvailablePipe.h"

namespace pdthr
{

MsgAvailablePipe::MsgAvailablePipe( void )
{
    int fds[2];

    // try to create the pipe
    if( ::pipe( fds ) == 0 )
    {
        // set the write end to be non-blocking
        //
        // A non-blocking pipe is a way to avoid the deadlock that
        // occurs in case the sender sends so many messages that it
        // fills the pipe causing the sender to block writing into the pipe
        // (and holding the receiver's mailbox's mutex), while the receiver
        // is blocked waiting to acquire the mutex in order to consume
        // the bytes from the pipe.
        int fret = fcntl( fds[1], F_GETFL );
        if( fret >= 0 )
        {
            fret = fcntl( fds[1], F_SETFL, fret | O_NONBLOCK );
        }

        if( fret >= 0 )
        {
            // save the pipe's endpoints
            rfd = PdFile( fds[0] );
            wfd = PdFile( fds[1] );
        }
        else
        {
            close( fds[0] );
            close( fds[1] );
        }
    }
}

MsgAvailablePipe::~MsgAvailablePipe( void )
{
    close( rfd.fd );
    close( wfd.fd );
}


bool
MsgAvailablePipe::Drain( void )
{
    bool ret = true;

	pollfd pfd;

	pfd.fd = rfd.fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	bool done = false;
	while( !done )
	{
		int pollret = ::poll( &pfd, 1, 0 );
		if( pollret == 1 )
		{
			// read a chunk
			static char buf[1024];
			read( rfd.fd, buf, 1024 );
		}
		else if( pollret == 0 )
		{
			// we've consumed all the data available on the msg wakeup pipe
			done = true;
		}
		else
		{
            // there was an error
            ret = false;
		}
	}
    return ret;
}


bool
MsgAvailablePipe::RaiseMsgAvailable( void )
{
    bool ret = true;

    static char s = '0';
    ssize_t nWritten = write( wfd.fd, &s, sizeof(char) );
    if( (nWritten == -1) && (errno == EAGAIN) )
    {
        // write failed - pipe is full
        ret = false;
    }
    return ret;
}

} // namespace pdthr

