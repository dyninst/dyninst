/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Mutex-pthread.C,v 1.5 2008/10/09 19:54:11 mjbrim Exp $
#include <assert.h>
#include <errno.h>
#include "Mutex-pthread.h"

namespace XPlat
{

Mutex::Mutex( void )
  : data( new PthreadMutexData )
{
    // nothing else to do
}

Mutex::~Mutex( void )
{
    delete data;
    data = NULL;
}

int Mutex::Lock( void )
{
    if( data != NULL )
        return data->Lock();
    else
        return EINVAL;
}

int Mutex::Unlock( void )
{
    if( data != NULL )
        return data->Unlock();
    else
        return EINVAL;
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

int PthreadMutexData::Lock( void )
{
    return pthread_mutex_lock( &mutex );
}

int PthreadMutexData::Unlock( void )
{
    return pthread_mutex_unlock( &mutex );
}

} // namespace XPlat
