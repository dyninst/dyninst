/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Monitor-win.h,v 1.2 2004/03/23 01:12:23 eli Exp $
//
// Declaration of the WinMonitor class.
// A WinMonitor is a Win32-based Monitor.
// 
// Note: the implementation for condition variables on Win32 used by
// this class is based mainly on an approach detailed in "Strategies for 
// Implementing POSIX Condition Variables on Win32" by Douglas C. Schmidt 
// and Irfan Pyarali, C++ Report, SIGS, Vol 10., No. 5, June 1998.
//
#ifndef XPLAT_WINMONITOR_H
#define XPLAT_WINMONITOR_H

#include <windows.h>
#include <map>
#include "xplat/Monitor.h"


namespace XPlat
{

class WinMonitorData : public Monitor::Data
{
private:
    class ConditionVariable
    {
    private:
        HANDLE hMutex;                      // owning monitor's mutex
        HANDLE hWaitSemaphore;              // semaphore on which threads wait
        HANDLE hAllWaitersReleasedEvent;    // event on which broadcaster waits
                                            // till all waiters have been 
                                            // released and run
        bool waitReleasedByBroadcast;       // indicates whether signal or
                                            // broadcast was used to 
                                            // release waiters

        unsigned int nWaiters;              // waiter count
        CRITICAL_SECTION nWaitersMutex;     // protects access to waiter count

    public:
        ConditionVariable( HANDLE _hMutex = NULL )
          : hMutex( _hMutex ),
            hWaitSemaphore( NULL ),
            hAllWaitersReleasedEvent( NULL ),
            waitReleasedByBroadcast( false ),
            nWaiters( 0 )
        {
            assert( hMutex != NULL );
        }
        ~ConditionVariable( void );

        int Init( void );
        int Wait( void );
        int Signal( void );
        int Broadcast( void );
    };

    typedef std::map<unsigned int, ConditionVariable*> ConditionVariableMap;


    HANDLE hMutex;              // handle to mutex object controlling 
                                // access to critical section
    ConditionVariableMap cvmap; // map of condition variables, indexed
                                // by user-chosen id
    bool locked;                // true iff we are in the critical section

public:
    WinMonitorData( void );
    virtual ~WinMonitorData( void );

    // methods dealing with the mutex
    virtual int Lock( void );
    virtual int Unlock( void );

    // methods dealing with condition variables
    virtual int RegisterCondition( unsigned int cvid );
    virtual int WaitOnCondition( unsigned int cvid );
    virtual int SignalCondition( unsigned int cvid );
    virtual int BroadcastCondition( unsigned int cvid );
};

} // namespace XPlat

#endif // XPLAT_WINMONITOR_H
