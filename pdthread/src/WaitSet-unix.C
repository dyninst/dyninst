#include "WaitSet-unix.h"

#define CURRENT_FILE WaitSet_unix_C
#include "thr_debug.h"


namespace pdthr
{

const unsigned int UnixWaitSet::kAllocationUnit = 16;

WaitSet*
WaitSet::BuildWaitSet( void )
{
    return new UnixWaitSet;
}


void
UnixWaitSet::Clear( void )
{
    delete[] fds;
    fds = NULL;
    nWaiters = 0;
}




bool
UnixWaitSet::HasData( int fd )
{
    bool ret = false;

    int idx = Find( fd );
    if( idx != -1 )
    {
        ret = (fds[idx].revents & POLLIN);
    }
    return ret;
}



void
UnixWaitSet::Add( int fd )
{
    int idx = Find( fd );
    if( idx == -1 )
    {
        // the file descriptor has not already been added to our set
        if( nWaiters == nAllocated )
        {
            // we've run out of room 

            // save info about the current allocation
            pollfd* oldfds = fds;
            unsigned int oldNumAllocated = nAllocated;

            // allocate a larger array of pollfd structs
            nAllocated += kAllocationUnit;
            fds = new pollfd[nAllocated];

            // copy old elements if necessary
            if( oldfds != NULL )
            {
                for( unsigned int i = 0; i < oldNumAllocated; i++ )
                {
                    fds[i].fd = oldfds[i].fd;
                }
                delete[] oldfds;
            }
        }
        fds[nWaiters].fd = fd;
        nWaiters++;
    }
}



int
UnixWaitSet::Find( int fd )
{
    for( unsigned int i = 0; i < nWaiters; i++ )
    {
        if( fds[i].fd == fd )
        {
            return i;
        }
    }
    return -1;
}


WaitSet::WaitReturn
UnixWaitSet::Wait( void )
{
    WaitSet::WaitReturn ret = WaitSet::WaitNone;

    if( nWaiters > 0 )
    {
        bool doneTrying = false;

        while( !doneTrying )
        {
            // set the events we're looking for
            for( unsigned int i = 0; i < nWaiters; i++ )
            {
                fds[i].events = POLLIN;
                fds[i].revents = 0;
            }

            // wait for indication of input
            int pret = poll( fds, nWaiters, -1 );
            if( pret > 0 )
            {
                // something has input
                doneTrying = true;
                ret = WaitSet::WaitInput;                            
            }
            else if( pret == 0 )
            {
                // the poll timed out - shouldn't happen (?)
                doneTrying = true;
                ret = WaitSet::WaitTimeout;
            }
            else
            {
                // error occurred with the poll call
                if( errno != EINTR )
                {
                    // the error was *not* due to interruption by a signal
                    // don't retry
                    doneTrying = true;
                    ret = WaitSet::WaitError;
                }
            }
        }
    }
    return ret;
}

} // namespace pdthr

