/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Mutex-pthread.C,v 1.1 2003/11/14 19:27:03 pcroth Exp $
#include <assert.h>
#include "xplat/src/Mutex-pthread.h"

namespace XPlat
{

Mutex::Mutex( void )
  : data( new PthreadMutexData )
{
    // nothing else to do
}

PthreadMutexData::PthreadMutexData( void )
  : initialized( false )
{
    int ret = pthread_mutex_init( &mutex, NULL );
    if( ret == 0 )
    {
        initialized = true;
    }
}

PthreadMutexData::~PthreadMutexData( void )
{
    if( initialized )
    {
        pthread_mutex_destroy( &mutex );
    }
}

} // namespace XPlat
