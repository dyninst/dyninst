#ifndef PDTHR_WAITSET_H
#define PDTHR_WAITSET_H

#include "pdutil/h/pdsocket.h"
#include "pdutil/h/pddesc.h"

namespace pdthr
{

class WaitSet
{
protected:
    WaitSet( void ) { }

public:
    enum WaitReturn
    {
        WaitNone,
        WaitTimeout,
        WaitError,
        WaitInput
    };

    static WaitSet* BuildWaitSet( void );

    virtual ~WaitSet( void ) { }

    virtual void Add( const PdSocket& s )       = NULL;
    virtual void Add( const PdFile& f )         = NULL;
    virtual void Clear( void )                  = NULL;
    virtual WaitReturn Wait( void )             = NULL;
    virtual bool HasData( const PdSocket& s )   = NULL;
    virtual bool HasData( const PdFile& f )     = NULL;
};

} // namespace pdthr

#endif // PDTHR_WAITSET_H
