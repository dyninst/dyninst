/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Mutex-pthread.h,v 1.4 2008/10/09 19:54:12 mjbrim Exp $
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


    virtual int Lock( void );
    virtual int Unlock( void );
};

} // namespace XPlat

#endif // PTHR_PTHREADMUTEX_H
