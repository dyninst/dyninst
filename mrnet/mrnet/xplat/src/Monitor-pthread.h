/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Monitor-pthread.h,v 1.4 2005/02/15 20:00:08 tlmiller Exp $
#ifndef XPLAT_PTHREADMONITOR_H
#define XPLAT_PTHREADMONITOR_H
#include <pthread.h>
#include <map>
#include "xplat/Monitor.h"


namespace XPlat
{

class PthreadMonitorData : public Monitor::Data
{
private:
    typedef std::map<unsigned int, pthread_cond_t*> ConditionVariableMap;

    pthread_mutex_t mutex;
    ConditionVariableMap cvmap;
    bool initialized;

public:
    PthreadMonitorData( void );
    virtual ~PthreadMonitorData( void );

    // methods dealing with the mutex
    virtual int Lock( void );
    virtual int Unlock( void );

    // methods dealing with condition variables
    virtual int RegisterCondition( unsigned int cvid );
    virtual int WaitOnCondition( unsigned int cvid );
    virtual int SignalCondition( unsigned int cvid );
    virtual int BroadcastCondition( unsigned int cvid );
};

} // namespace XPlat

#endif // XPLAT_PTHREADMONITOR_H
