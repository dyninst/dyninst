/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Mutex-pthread.h,v 1.1 2003/11/14 19:27:03 pcroth Exp $
#ifndef XPLAT_PTHREADMUTEX_H
#define XPLAT_PTHREADMUTEX_H

#include <pthread.h>
#include "xplat/Mutex.h"


namespace XPlat
{

class PthreadMutexData : public Mutex::Data
{
private:
    pthread_mutex_t mutex;
    bool initialized;

public:
    PthreadMutexData( void );
    virtual ~PthreadMutexData( void );


    virtual int Lock( void )
    {
        return pthread_mutex_lock( &mutex );
    }

    virtual int Unlock( void )
    {
        return pthread_mutex_unlock( &mutex );
    }
};

} // namespace XPlat

#endif // PTHR_PTHREADMUTEX_H
