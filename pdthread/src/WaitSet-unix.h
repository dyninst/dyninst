#ifndef PDTHR_UNIXWAITSET_H
#define PDTHR_UNIXWAITSET_H


#include <poll.h>
#include "pdthread/src/WaitSet.h"


namespace pdthr
{

class UnixWaitSet : public WaitSet
{
private:
    static const unsigned int kAllocationUnit;

    pollfd* fds;
    unsigned int nWaiters;
    unsigned int nAllocated;


    int Find( int fd );
    void Add( int fd );
    bool HasData( int fd );

public:
    UnixWaitSet( void )
      : fds( NULL ),
        nWaiters( 0 ),
        nAllocated( 0 )
    { }
    virtual ~UnixWaitSet( void )                { delete[] fds; }

    virtual void Clear( void );
    virtual void Add( const PdSocket& s )       { Add( s.s ); }
    virtual void Add( const PdFile& f )         { Add( f.fd ); }
    virtual WaitSet::WaitReturn Wait( void );
    virtual bool HasData( const PdSocket& s )   { return HasData( s.s ); }
    virtual bool HasData( const PdFile& f )     { return HasData( f.fd ); }
};

} // namespace pdthr

#endif // PDTHR_UNIXWAITSET_H
