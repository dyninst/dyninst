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

