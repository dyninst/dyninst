/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Mutex-win.h,v 1.1 2003/11/14 19:27:03 pcroth Exp $
#ifndef XPLAT_WINMUTEX_H
#define XPLAT_WINMUTEX_H

#include <windows.h>
#include "xplat/Mutex.h"


namespace XPlat
{

class WinMutexData : public Mutex::Data
{
private:
    CRITICAL_SECTION mutex;

public:
    WinMutexData( void )
    {
        InitializeCriticalSection( &mutex );
    }

    virtual ~WinMutexData( void )
    {
        DeleteCriticalSection( &mutex );
    }

    virtual int Lock( void )
    {
        EnterCriticalSection( &mutex );
        return 0;
    }

    virtual int Unlock( void )
    {
        LeaveCriticalSection( &mutex );
        return 0;
    }
};

} // namespace XPlat

#endif // PTHR_WINMUTEX_H
