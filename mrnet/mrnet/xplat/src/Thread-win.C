/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Thread-win.C,v 1.4 2008/10/09 19:53:55 mjbrim Exp $
#include <windows.h>
#include <assert.h>
#include "xplat/Thread.h"

namespace XPlat
{

int
Thread::Create( Func func, void* data, Id* id )
{
    assert( id != NULL );

    // create the thread
    HANDLE hThread = CreateThread( NULL,   // security attributes
                                    0,      // default stack size
                                    (LPTHREAD_START_ROUTINE)func,   // start routine
                                    data,   // arg to start routine
                                    0,      // creation flags
                                    NULL ); // loc to store thread id (unused)
    if( hThread != NULL )
    {
        *id = (Thread::Id)hThread;
    }
    return (hThread == NULL) ? -1 : 0;
}


Thread::Id
Thread::GetId( void )
{
    return (Id)(GetCurrentThread());
}


int
Thread::Join( Id joinWith, void** exitValue )
{
    int ret = -1;

    // wait for the indicated thread to exit
    //
    // TODO yes, we know this doesn't handle all possible return values
    // from WaitForSingleObject.
    //
    if( WaitForSingleObject( (HANDLE)joinWith, INFINITE ) == WAIT_OBJECT_0 )
    {
        if( exitValue != NULL )
        {
            // extract departed thread's exit code
            GetExitCodeThread( (HANDLE)joinWith, (LPDWORD)exitValue );
        }
        ret = 0;
    }
    return ret;
}

int Thread::Cancel( Id id )
{
    if( TerminateThread( (HANDLE)id, (DWORD)0) )
	    return 0;
    else
	    return (int) GetLastError();
}


void
Thread::Exit( void* val )
{
    ExitThread( (DWORD)val );
}


} // namespace XPlat

