/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Monitor.h,v 1.2 2004/03/23 01:12:22 eli Exp $
#ifndef XPLAT_MONITOR_H
#define XPLAT_MONITOR_H

#if !defined(NULL)
#include <stdlib.h>
#endif // !defined(NULL)

namespace XPlat
{

class Monitor
{
public:
    class Data
    {
    public:
        Data( void ) { }
        virtual ~Data( void ) { }

        // critical section-related methods
        virtual int Lock( void ) = NULL;
        virtual int Unlock( void ) = NULL;

        // condition variable-related methods
        virtual int RegisterCondition( unsigned int condid ) = NULL;
        virtual int WaitOnCondition( unsigned int condid ) = NULL;
        virtual int SignalCondition( unsigned int condid ) = NULL;
        virtual int BroadcastCondition( unsigned int condid ) = NULL;
    };

private:
    Data* data;

public:
    Monitor( void );
    virtual ~Monitor( void )            { delete data; }

    // critical section-related methods
    virtual int Lock( void )            { return data->Lock(); }
    virtual int Unlock( void )          { return data->Unlock(); }

    // condition variable-related methods
    virtual int RegisterCondition( unsigned int condid )
    {
        return data->RegisterCondition( condid );
    }

    virtual int WaitOnCondition( unsigned int condid )
    {
        return data->WaitOnCondition( condid );
    }

    virtual int SignalCondition( unsigned int condid )
    {
        return data->SignalCondition( condid );
    }

    virtual int BroadcastCondition( unsigned int condid )
    {
        return data->BroadcastCondition( condid );
    }
};

} // namespace XPlat

#endif // XPLAT_MONITOR_H
