/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Once-pthread.h,v 1.1 2003/11/14 19:27:03 pcroth Exp $
#ifndef XPLAT_PTHREADONCE_H
#define XPLAT_PTHREADONCE_H

#include <pthread.h>
#include "xplat/Once.h"

namespace XPlat
{

class PthreadOnceData : public Once::Data
{
private:
    pthread_once_t latch;

public:
    PthreadOnceData( void )
    {
        // On some platforms, pthread_once_t is a struct containing
        // a vector, so initializing latch using the PTHREAD_ONCE_INIT
        // value can be tricky.
        // We can't initialize it in the constructor's initializer list,
        // nor can we just assign PTHREAD_ONCE_INIT to latch once
        // it has been constructed.
        // We assume that it is OK to copy one pthread_once_t to another,
        // and initialize a temporary one with PTHREAD_ONCE_INIT.
        pthread_once_t latchval = PTHREAD_ONCE_INIT;
        latch = latchval;
    }
    virtual int DoOnce( void (*func)( void ) );
};

} // namespace XPlat

#endif // XPLAT_PTHREADONCE_H
