/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Mutex-pthread.C,v 1.3 2005/03/21 18:58:28 darnold Exp $
#include <assert.h>
#include "Mutex-pthread.h"

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
