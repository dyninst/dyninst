#ifndef PDTHR_DUMMYMONITOR_H
#define PDTHR_DUMMYMONITOR_H

#include "xplat/Monitor.h"

namespace pdthr
{

class DummyMonitor : public XPlat::Monitor
{
public:
    virtual ~DummyMonitor( void ) { }

    // critical section-related methods
    virtual int Lock( void ) { return 0; }
    virtual int Unlock( void ) { return 0; }

    // condition variable-related methods
    virtual int RegisterCondition( unsigned int condid ) { return 0; }
    virtual int WaitOnCondition( unsigned int condid ) { return 0; }
    virtual int SignalCondition( unsigned int condid ) { return 0; }
    virtual int BroadcastCondition( unsigned int condid ) { return 0; }
};

} // namespace pdthr

#endif // PDTHR_DUMMYMONITOR_H
