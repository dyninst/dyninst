/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Monitor-win.C,v 1.3 2005/03/14 21:12:48 gquinn Exp $
//
// Implementation of the WinMonitor class.
// A WinMonitor is a Win32-based Monitor.
// 
// Note: the implementation for condition variables on Win32 used by
// this class is based mainly on an approach detailed in "Strategies for 
// Implementing POSIX Condition Variables on Win32" by Douglas C. Schmidt 
// and Irfan Pyarali, C++ Report, SIGS, Vol 10., No. 5, June 1998.
//
#include <assert.h>
#include <limits.h>
#include "xplat/src/Monitor-win.h"

namespace XPlat
{

Monitor::Monitor( void )
  : data( new WinMonitorData )
{
    // nothing else to do
}


WinMonitorData::WinMonitorData( void )
  : hMutex( CreateMutex( NULL, FALSE, NULL ) )
{
    // nothing else to do
}



WinMonitorData::~WinMonitorData( void )
{
    // release any condition variables that have been defined
    for( ConditionVariableMap::iterator iter = cvmap.begin();
            iter != cvmap.end();
            iter++ )
    {
        delete iter->second;
    }
    cvmap.clear();

    if( hMutex != NULL )
    {
        // release the Mutex
        CloseHandle( hMutex );
    }
}


int
WinMonitorData::Lock( void )
{
    assert( hMutex != NULL );
    DWORD dwRet = WaitForSingleObject( hMutex, INFINITE );
    return ((dwRet == WAIT_OBJECT_0) ? 0 : -1);
}


int
WinMonitorData::Unlock( void )
{
    assert( hMutex != NULL );

    ReleaseMutex( hMutex );

    return 0;
}



int
WinMonitorData::RegisterCondition( unsigned int cvid )
{
    int ret = -1;

    assert( hMutex != NULL );

    // check if there is already a condition associated with the chosen id
    if( cvmap[cvid] == NULL )
    {
        // there was no condition already associated with this condition var
        // build one
        ConditionVariable* newcv = new ConditionVariable( hMutex );

        // initialize the condition variable
        if( newcv->Init() == 0 )
        {
            cvmap[cvid] = newcv;
            ret = 0;
        }
        else
        {
            // failed to initialized - release the object
            delete newcv;
        }
    }
    return ret;
}


int
WinMonitorData::WaitOnCondition( unsigned int cvid )
{
    int ret = -1;

    assert( hMutex != NULL );

    ConditionVariableMap::iterator iter = cvmap.find( cvid );
    if( iter != cvmap.end() )
    {
        ConditionVariable* cv = cvmap[cvid];
        assert( cv != NULL );

        ret = cv->Wait();
    }
    else
    {
        // bad cvid
        // TODO how to indicate the error?
	assert(0);
    }
    return ret;
}


int
WinMonitorData::SignalCondition( unsigned int cvid )
{
    int ret = -1;

    assert( hMutex != NULL );
 
    ConditionVariableMap::iterator iter = cvmap.find( cvid );
    if( iter != cvmap.end() )
    {
        ConditionVariable* cv = cvmap[cvid];
        assert( cv != NULL );

        ret = cv->Signal();
    }
    else
    {
        // bad cvid
        // TODO how to indicate the error?
    }
    return ret;
}


int
WinMonitorData::BroadcastCondition( unsigned int cvid )
{
    int ret = -1;

    assert( hMutex != NULL );

    ConditionVariableMap::iterator iter = cvmap.find( cvid );
    if( iter != cvmap.end() )
    {
        ConditionVariable* cv = cvmap[cvid];
        assert( cv != NULL );

        ret = cv->Broadcast();
    }
    else
    {
        // bad cvid
        // TODO how to indicate the error?
    }
    return ret;
}


//----------------------------------------------------------------------------
// WinMonitorData::ConditionVariable methods
//----------------------------------------------------------------------------

WinMonitorData::ConditionVariable::~ConditionVariable( void )
{
    if( hWaitSemaphore != NULL )
    {
        CloseHandle( hWaitSemaphore );
    }

    if( hAllWaitersReleasedEvent != NULL )
    {
        CloseHandle( hAllWaitersReleasedEvent );
    }

    DeleteCriticalSection( &nWaitersMutex );
}


int
WinMonitorData::ConditionVariable::Init( void )
{
    // We use two Win32 synchronization objects to implement condition 
    // variables, as modelled by the "Strategies..." article cited at the top
    // of this file.  We use a Semaphore on which threads can wait for the
    // condition to be signalled/broadcast.  For fairness, we also use an 
    // Event so that the waiters, once released, can indicate to the 
    // broadcaster that they have all been released. 


    // Semaphore on which threads wait for the condition to be signalled.
    //
    hWaitSemaphore = CreateSemaphore( NULL,         // default security attrs
                                        0,          // initial count
                                        LONG_MAX,   // max count
                                        NULL );     // no name needed


    // The all-waiters released event is used in case a thread uses a broadcast
    // to release all waiting threads.  After the broadcaster releases all
    // the waiting threads, it blocks itself on this event till the last
    // waiter is known to have been awakened.
    //
    hAllWaitersReleasedEvent = CreateEvent( NULL,   // default security attrs
                                            FALSE,  // make auto-reset event
                                            FALSE,  // initially non-signalled
                                            NULL ); // no name needed

    InitializeCriticalSection( &nWaitersMutex );

    return (((hWaitSemaphore != NULL) && (hAllWaitersReleasedEvent != NULL)) ?
            0 : -1);
}



int
WinMonitorData::ConditionVariable::Wait( void )
{
    int ret = -1;
    DWORD dwret;        // used for return value from Win32 calls


    // We're going to be waiting, so bump the number of waiters.
    // Even though we currently have our owning Monitor's mutex locked,
    // we need to protect access to this variable because our access to 
    // this variable later in this file occur when we do *not* have it locked.
    EnterCriticalSection( &nWaitersMutex );
    nWaiters++;
    LeaveCriticalSection( &nWaitersMutex );

    // Atomically release our owning object's mutex and 
    // wait on our semaphore.  ("Signalling" a Win32 Mutex releases it.)
    dwret = SignalObjectAndWait( hMutex,            // obj to signal
                                    hWaitSemaphore, // obj to wait on
                                    INFINITE,   // timeout
                                    FALSE );    // alertable?
    if( dwret == WAIT_OBJECT_0 )
    {
        // We have been released from the semaphore.
        // At this point, we do *not* have the critical section mutex.

        EnterCriticalSection( &nWaitersMutex );
        nWaiters--;

        // check if we were released via a broadcast and we are
        // the last thread to be released
        bool signalBroadcaster = (waitReleasedByBroadcast && (nWaiters == 0));
        LeaveCriticalSection( &nWaitersMutex );

        if( signalBroadcaster )
        {
            // We were unblocked by a broadcast and we were the last
            // waiter to run.  We need to:
            //
            //   1. Signal the broadcaster that it should continue (and
            //      release the owning Monitor's mutex lock)
            //   2. Try to acquire the owning Monitor's mutex lock.
            //
            // We need to do both in the same atomic operation.
            dwret = SignalObjectAndWait( hAllWaitersReleasedEvent,  // obj to signal
                                            hMutex,         // obj to wait on
                                            INFINITE,       // timeout
                                            FALSE );        // alertable?
        }
        else
        {
            // We were unblocked by a signal operation, or we were not
            // the last waiter to run.  We just need to try to reacquire the
            // owning Monitor's mutex lock.
            dwret = WaitForSingleObject( hMutex, INFINITE );
        }

        if( dwret == WAIT_OBJECT_0 )
        {
            ret = 0;
        }
        else
        {
            // the wait or signalandwait failed
            // TODO how to indicate this to the user?
        }
    }
    else
    {
        // SignalObjectAndWait failed
        // TODO how to indicate the failure?
    }

    return ret;
}



int
WinMonitorData::ConditionVariable::Signal( void )
{
    int ret = 0;

    // Check if there are any waiters to signal.
    EnterCriticalSection( &nWaitersMutex );
    bool anyWaiters = (nWaiters > 0);
    LeaveCriticalSection( &nWaitersMutex );

    // release one thread from the Semaphore if necessary
    if( anyWaiters )
    {
        if( !ReleaseSemaphore( hWaitSemaphore,
                                1,              // release one thread
                                NULL ) )        // don't care about prev value
        {
            // we failed to release a thread
            // TODO how to report more complete error to user?
            ret = -1;
        }
    }
    return ret;
}


int
WinMonitorData::ConditionVariable::Broadcast( void )
{
    int ret = 0;

    // Check if there are any waiters.
    EnterCriticalSection( &nWaitersMutex );
    bool haveWaiters = (nWaiters > 0);
    if( haveWaiters )
    {
        waitReleasedByBroadcast = true;
        
        // release all waiters atomically
        if( !ReleaseSemaphore( hWaitSemaphore,
                                    nWaiters,       // number to release
                                    NULL ) )        // don't care about prev val
        {
            // TODO how to give more error info to user?
            ret = -1;
        }
    }
    LeaveCriticalSection( &nWaitersMutex );

    // Wait for last waiter to tell us it has been unblocked and run
    // (if there were any waiters to be released).
    if( haveWaiters && (ret == 0) )
    {
        DWORD dwret = WaitForSingleObject( hAllWaitersReleasedEvent, INFINITE );
        if( dwret != WAIT_OBJECT_0 )
        {
            // TODO how to give more error info to user?
            ret = -1;
        }
        waitReleasedByBroadcast = false;
    }

    return ret;
}

} // namespace XPlat

