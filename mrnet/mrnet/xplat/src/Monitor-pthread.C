/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Monitor-pthread.C,v 1.7 2008/10/09 19:54:09 mjbrim Exp $
#include <assert.h>
#include <errno.h>
#include "Monitor-pthread.h"

namespace XPlat
{

Monitor::Monitor( void )
  : data( new PthreadMonitorData )
{
    // nothing else to do
}

Monitor::~Monitor( void )
{
    delete data;
    data = NULL;
}

int Monitor::Lock( void )
{
    if( data != NULL )
        return data->Lock();
    return EINVAL;
}

int Monitor::Unlock( void )
{
    if( data != NULL )
        return data->Unlock();
    return EINVAL;
}

PthreadMonitorData::PthreadMonitorData( void )
  : initialized( false )
{
	int ret;
    pthread_mutexattr_t recursive;
    
    ret = pthread_mutexattr_init( & recursive );
    assert( ret == 0 );
    ret = pthread_mutexattr_settype( & recursive, PTHREAD_MUTEX_RECURSIVE );
    assert( ret == 0 );

    ret = pthread_mutex_init( & mutex, & recursive );
    assert( ret == 0 );
    initialized = true;
}


PthreadMonitorData::~PthreadMonitorData( void )
{
    if( initialized )
    {
        for( ConditionVariableMap::iterator iter = cvmap.begin();
                iter != cvmap.end();
                iter++ )
        {
            pthread_cond_destroy( iter->second );
        }
        cvmap.clear();

        pthread_mutex_destroy( &mutex );
    }
}


int
PthreadMonitorData::Lock( void )
{
    return pthread_mutex_lock( &mutex );
}


int
PthreadMonitorData::Unlock( void )
{
    return pthread_mutex_unlock( &mutex );
}


int
PthreadMonitorData::RegisterCondition( unsigned int cvid )
{
    int ret = 0;

    pthread_cond_t* newcv = new pthread_cond_t;
    if( pthread_cond_init( newcv, NULL ) == 0 )
    {
        cvmap[cvid] = newcv;
    }
    else
    {
        delete newcv;
        ret = -1;
    }
    return ret;
}


int
PthreadMonitorData::WaitOnCondition( unsigned int cvid )
{
    int ret = -1;

    ConditionVariableMap::iterator iter = cvmap.find( cvid );
    if( iter != cvmap.end() )
    {
        pthread_cond_t* cv = cvmap[cvid]; 
        assert( cv != NULL );
        ret = pthread_cond_wait( cv, &mutex );
    }
    else
    {
        // bad condition variable id
        // TODO how to indicate the error
    	assert( 0 );
    }
    return ret;
}


int
PthreadMonitorData::SignalCondition( unsigned int cvid )
{
    int ret = -1;

    ConditionVariableMap::iterator iter = cvmap.find( cvid );
    if( iter != cvmap.end() )
    {
        pthread_cond_t* cv = cvmap[cvid];
        assert( cv != NULL );
        ret = pthread_cond_signal( cv );
    }
    else
    {
        // bad condition variable id
        // TODO how to indicate the error?
    }
    return ret;
}


int
PthreadMonitorData::BroadcastCondition( unsigned int cvid )
{
    int ret = -1;

    ConditionVariableMap::iterator iter = cvmap.find( cvid );
    if( iter != cvmap.end() )
    {
        pthread_cond_t* cv = cvmap[cvid];
        assert( cv != NULL );
        ret = pthread_cond_broadcast( cv );
    }
    else
    {
        // bad condition variable id
        // TODO how to indicate the error?
    }
    return ret;
}

} // namespace XPlat

