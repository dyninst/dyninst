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
        if(!fds || (nWaiters == nAllocated) )
        {
            // we've run out of room 

            // save info about the current allocation
            pollfd* oldfds = fds;
            unsigned int oldNumAllocated = nAllocated;

            // allocate a larger array of pollfd structs
	    if (nWaiters == nAllocated)
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


WaitSet::WaitReturn UnixWaitSet::Wait( void )
{
   WaitSet::WaitReturn ret = WaitSet::WaitNone;
   
   if( nWaiters > 0 )
   {
      bool doneTrying = false;
      int timeout = -1;
      bool *data_available = (bool *) malloc(nWaiters * sizeof(bool));
      while( !doneTrying )
      {
         // set the events we're looking for
         for( unsigned int i = 0; i < nWaiters; i++ )
         {
            if (PollCallback && PollCallback(fds[i].fd))
            {
               timeout = 0;
               data_available[i] = true;		
            }
            else
               data_available[i] = false;
            fds[i].events = POLLIN;
            fds[i].revents = 0;
         }

         // wait for indication of input
         int pret = poll(fds, nWaiters, timeout);
         if( pret > 0 || (pret == 0 && timeout == 0))
         {
            for ( unsigned i = 0; i < nWaiters; i++)
               if (data_available[i]) fds[i].revents |= POLLIN; 
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
	free(data_available);
    }
    return ret;
}

} // namespace pdthr

