#ifndef WAITSET_WIN_H
#define WAITSET_WIN_H

#include "common/h/Vector.h"
#include "pdthread/src/WaitSet.h"

namespace pdthr
{

class WinWaitSet : public WaitSet
{
private:
    bool check_wmsg_q;
    pdvector<PdSocket> socks;
    pdvector<PdFile> files;

    bool wmsg_q_hasdata;
    int readySockIdx;
    int readyFileIdx;

public:
    WinWaitSet( void );
    virtual ~WinWaitSet( void );

    virtual void Add( const PdSocket& s )   { socks.push_back( s ); }
    virtual void Add( const PdFile& f )     { files.push_back( f ); }
    virtual void Clear( void );
    virtual WaitReturn Wait( void );
    virtual bool HasData( const PdSocket& s );
    virtual bool HasData( const PdFile& f );

    virtual void AddWmsgQueue( void )       { check_wmsg_q = true; }
    virtual bool WmsgQueueHasData( void )   { return wmsg_q_hasdata; }
};

} // namespace pdthr

#endif // WAITSET_WIN_H
