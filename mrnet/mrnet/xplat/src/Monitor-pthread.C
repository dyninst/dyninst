/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Monitor-pthread.C,v 1.1 2003/11/14 19:27:02 pcroth Exp $
#include <assert.h>
#include "xplat/src/Monitor-pthread.h"


namespace XPlat
{

Monitor::Monitor( void )
  : data( new PthreadMonitorData )
{
    // nothing else to do
}


PthreadMonitorData::PthreadMonitorData( void )
  : initialized( false ),
    locked( false )
{
    int ret = pthread_mutex_init( &mutex, NULL );
    if( ret == 0 )
    {
        initialized = true;
    }
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
    int ret = pthread_mutex_lock( &mutex );
    if( ret == 0 )
    {
        locked = true;
    }
    return ret;
}


int
PthreadMonitorData::Unlock( void )
{
    assert( locked );
    locked = false;
    int ret = pthread_mutex_unlock( &mutex );
    if( ret != 0 )
    {
        locked = true;
    }
    return ret;
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

    assert( locked );
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
    }
    return ret;
}


int
PthreadMonitorData::SignalCondition( unsigned int cvid )
{
    int ret = -1;

    assert( locked );
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

    assert( locked );
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

